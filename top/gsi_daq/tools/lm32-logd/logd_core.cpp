/*!
 *  @file logd_core.cpp
 *  @brief Main functionality of LM32 log daemon.
 *
 *  @date 21.04.2022
 *  @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#ifndef __DOCFSM__
 #include <iomanip>
 #include <scu_mmu_tag.h>
 #include <daq_calculations.hpp>
 #include <daqt_messages.hpp>
 #include "logd_core.hpp"
#endif
using namespace Scu;
using namespace std;

constexpr uint LM32_OFFSET = 0x10000000;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
Lm32Logd::Lm32Logd( mmuEb::EtherboneConnection& roEtherbone, CommandLine& rCmdLine )
   :m_rCmdLine( rCmdLine )
   ,m_oMmu( &roEtherbone )
   ,m_lastTimestamp( 0 )
   ,m_maxItems( 10 )
   ,m_pMiddleBuffer( nullptr )
{
   if( !m_oMmu.isPresent() )
   {
      throw std::runtime_error( "MMU not present!" );
   }

   mmu::MMU_STATUS_T status = m_oMmu.allocate( mmu::TAG_LM32_LOG, m_offset, m_capacity );
   if( status != mmu::OK )
   {
      throw std::runtime_error( m_oMmu.status2String( status ) );
   }

   if( m_rCmdLine.isVerbose() )
   {
      cout << "Found MMU-tag:  0x" << hex << uppercase << setw( 4 ) << setfill('0')
           << mmu::TAG_LM32_LOG << dec
           << "\nAddress:        " << m_offset
           << "\nCapacity:       " << m_capacity << endl;
   }

   m_fifoAdminBase = m_offset * sizeof(mmu::RAM_PAYLOAD_T) + m_oMmu.getBase();

   m_offset   += SYSLOG_FIFO_ADMIN_SIZE;
   m_capacity -= SYSLOG_FIFO_ADMIN_SIZE;

   if( m_rCmdLine.isVerbose() )
   {
      cout << "Begin:          " << m_offset
         << "\nMax. log items: " << m_capacity / SYSLOG_FIFO_ITEM_SIZE << endl;
   }

   m_fiFoAdmin.admin.indexes.offset   = 0;
   m_fiFoAdmin.admin.indexes.capacity = 0;
   m_fiFoAdmin.admin.indexes.start    = 0;
   m_fiFoAdmin.admin.indexes.end      = 0;
   m_fiFoAdmin.admin.wasRead          = 0;
   updateFiFoAdmin( m_fiFoAdmin );

   if( m_rCmdLine.isVerbose() )
   {
      cout << "At the moment "
           << sysLogFifoGetItemSize( &m_fiFoAdmin )
           << " Log-items in FiFo." << endl;
   }

   m_lm32Base = m_oMmu.getEb()->findDeviceBaseAddress( mmuEb::gsiId,
                                                       mmuEb::lm32_ram_user );

   /*
    * The string addresses of LM32 comes from the perspective of the LM32.
    * Therefore a offset correction has to made here.
    */
   if( m_lm32Base < LM32_OFFSET )
   {
      throw std::runtime_error( "LM32 base address is corrupt!" );
   }
   m_lm32Base -= LM32_OFFSET;
}

/*! ---------------------------------------------------------------------------
 */
Lm32Logd::~Lm32Logd( void )
{
   if( m_pMiddleBuffer != nullptr )
      delete[] m_pMiddleBuffer;
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::operator()( void )
{
  // std::string str;
  // uint n = readStringFromLm32( str, 0x100020f8 );
  // std::cout << "Zeichen: " << n << ", Text: " << str << std::endl;
   readItems();
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::updateFiFoAdmin( SYSLOG_FIFO_ADMIN_T& rAdmin )
{
   assert( m_oMmu.getEb()->isConnected() );

   constexpr uint LEN32 = sizeof(SYSLOG_FIFO_ADMIN_T) / sizeof(uint32_t);

   m_oMmu.getEb()->read( m_fifoAdminBase, &rAdmin, EB_DATA32 | EB_LITTLE_ENDIAN, LEN32 );
   if( (rAdmin.admin.indexes.offset   != m_offset) ||
       (rAdmin.admin.indexes.capacity != m_capacity) )
   {
      throw std::runtime_error( "LM32 syslog fifo is corrupt!" );
   }
}

constexpr uint HIGHST_ADDR = 2 * LM32_OFFSET;

/*! ---------------------------------------------------------------------------
 */
uint Lm32Logd::readStringFromLm32( std::string& rStr, uint addr )
{
   if( !gsi::isInRange( addr, LM32_OFFSET, HIGHST_ADDR ) )
   {
      throw std::runtime_error( "String address is corrupt!" );
   }

   char buffer[16];
   uint ret = 0;

   while( true )
   {
      readLm32( buffer, sizeof( buffer ), addr );
      for( uint i = 0; i < sizeof( buffer ); i++ )
      {
         if( (buffer[i] == '\0') || (addr + i >= HIGHST_ADDR) )
            return ret;

         if( !m_rCmdLine.isForConsole() && ((buffer[i] == '\n') || (buffer[i] == '\r')) )
         {
            if( buffer[i] == '\n')
               rStr += ' ';
         }
         else
            rStr += buffer[i];
         ret++;
      }
      addr += sizeof( buffer );
   }
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::readItems( void )
{
   SYSLOG_FIFO_ADMIN_T fifoAdmin;

   updateFiFoAdmin( fifoAdmin );
   if( fifoAdmin.admin.wasRead != 0 )
      return;

   uint size = sysLogFifoGetSize( &fifoAdmin );
   if( size == 0 )
      return;

   if( (size % SYSLOG_FIFO_ITEM_SIZE) != 0 )
      return;

   m_fiFoAdmin = fifoAdmin;

   const uint readTotalLen = min( size, m_maxItems );
   uint len = readTotalLen;
   const uint numOfItems = readTotalLen / SYSLOG_FIFO_ITEM_SIZE;
   assert( m_pMiddleBuffer == nullptr );
   m_pMiddleBuffer = new SYSLOG_FIFO_ITEM_T[numOfItems*10]; //TODO: WHY??
   SYSLOG_FIFO_ITEM_T* pData = m_pMiddleBuffer;
   uint lenToEnd = sysLogFifoGetUpperReadSize( &m_fiFoAdmin );
   if( lenToEnd < readTotalLen )
   {
      readItems( pData, lenToEnd );
      pData += lenToEnd;
      len   -= lenToEnd;
   }
   readItems( pData, len );

   for( uint i = 0; i < numOfItems; i++ )
   {
      std::string output;
      evaluateItem( output, m_pMiddleBuffer[i] );
      cout << output << std::endl;
   }

   delete[] m_pMiddleBuffer;
   m_pMiddleBuffer = nullptr;
}

/*! ---------------------------------------------------------------------------
 */
inline bool Lm32Logd::isPaddingChar( const char c )
{
   switch( c )
   {
      case '0': /* No break here! */
      case ' ': /* No break here! */
      case '.': /* No break here! */
      case '_':
      {
         return true;
      }
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 */
inline bool Lm32Logd::isDecDigit( const char c )
{
   return gsi::isInRange( c, '0', '9');
}

#define FSM_DECLARE_STATE( newState, attr... ) newState
#define FSM_INIT_FSM( initState, attr... ) STATE_T state = initState
#define FSM_TRANSITION( target, attr... ) { state = target; break; }
#define FSM_TRANSITION_NEXT( target, attr... ) { state = target; next = true; break; }
#define FSM_TRANSITION_SELF( attr...) break

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::evaluateItem( std::string& rOutput, const SYSLOG_FIFO_ITEM_T& item )
{
   if( m_lastTimestamp >= item.timestamp )
   {
      ERROR_MESSAGE( "Invalid timestamp: " << item.timestamp );
      return;
   }

   m_lastTimestamp = item.timestamp;

   std::string format;
   readStringFromLm32( format, item.format );

   if( !m_rCmdLine.isNoTimestamp() )
   {
      if( m_rCmdLine.isHumanReadableTimestamp() )
      {
         rOutput += daq::wrToTimeDateString( item.timestamp );
         rOutput += std::to_string(item.timestamp % daq::NANOSECS_PER_SEC);
      }
      else
      {
         rOutput += std::to_string(item.timestamp);
      }
      rOutput += ": ";
   }

   enum STATE_T
   {
      FSM_DECLARE_STATE( NORMAL, color=blue ),
      FSM_DECLARE_STATE( PADDING_CHAR, color=green ),
      FSM_DECLARE_STATE( PADDING_SIZE, color=cyan ),
      FSM_DECLARE_STATE( PARAM, color=magenta )
   };

   FSM_INIT_FSM( NORMAL, color=blue );
   char paddingChar = ' ';
   uint paddingSize = 0;

   uint ai = 0;
   uint base = 10;
   for( uint i = 0; i < format.length(); i++ )
   {
      bool next;
      do
      {
         next = false;
         switch( state )
         {
            case NORMAL:
            {
               if( format[i] == '%' && (ai < ARRAY_SIZE(item.param)) )
                  FSM_TRANSITION( PADDING_CHAR, label='char == %', color=green );

               rOutput += format[i];

               FSM_TRANSITION_SELF( color=blue );
            }

            case PADDING_CHAR:
            {
               if( format[i] == '%' )
               {
                  rOutput += format[i];
                  FSM_TRANSITION( NORMAL, label='char == %', color=blue );
               }
               if( isPaddingChar( format[i] ) )
               {
                  paddingChar = format[i];
                  FSM_TRANSITION( PADDING_SIZE, color=cyan );
               }
               if( isDecDigit( format[i] ) )
               {
                  FSM_TRANSITION_NEXT( PADDING_SIZE  );
               }
               FSM_TRANSITION_NEXT( PARAM, color=magenta );
            }

            case PADDING_SIZE:
            {
               if( !isDecDigit( format[i] ) )
                  FSM_TRANSITION_NEXT( PARAM, color=magenta );
               paddingSize *= 10;
               paddingSize += format[i] - '0';
               FSM_TRANSITION_SELF( color=cyan );
            }

            case PARAM:
            {
               bool signum  = false;
               bool unknown = false;
               bool done    = false;
               unsigned char hexOffset = 0;
               assert( ai < ARRAY_SIZE(item.param) );
               switch( format[i] )
               {
                  case 'S': /* No break here! */
                  case 's':
                  {
                     if( gsi::isInRange( item.param[ai], LM32_OFFSET, HIGHST_ADDR ) )
                        readStringFromLm32( rOutput, item.param[ai] );
                     else
                        ERROR_MESSAGE( "String address of parameter " << (ai+1)
                                       << " is invalid: 0x"
                                       << std::hex  << std::uppercase << std::setfill('0')
                                       << std::setw( 8 ) << item.param[ai] << std::dec << " !" );
                     ai++;
                     done = true;
                     break;
                  }
                  case 'c':
                  {
                     rOutput += item.param[ai++];
                     done = true;
                     break;
                  }
                  case 'X':
                  {
                     base = 16;
                     hexOffset = 'A' - '9' - 1;
                     break;
                  }
                  case 'x':
                  {
                     base = 16;
                     hexOffset = 'a' - '9' - 1;
                     break;
                  }
                  case 'p':
                  {
                     base = 16;
                     hexOffset = 'A' - '9' - 1;
                     paddingChar = '0';
                     paddingSize = sizeof(uint32_t) * 2;
                     break;
                  }
                  case 'i': /* No break here! */
                  case 'd':
                  {
                     base = 10;
                     signum = true;
                     break;
                  }
                  case 'u':
                  {
                     base = 10;
                     break;
                  }
                  case 'o':
                  {
                     base = 8;
                     break;
                  }
             #ifndef CONFIG_NO_BINARY_PRINTF_FORMAT
                  case 'b':
                  {
                     base = 2;
                     break;
                  }
             #endif
                  default:
                  {
                     unknown = true;
                     break;
                  }
               }
               if( unknown )
                  FSM_TRANSITION_NEXT( NORMAL, color=blue );

               if( done )
                  FSM_TRANSITION( NORMAL, color=blue );

               uint32_t value = item.param[ai++];
               if( signum && ((value & (1 << (BIT_SIZEOF(value)-1))) != 0) )
               {
                  if( paddingSize > 0 )
                     paddingSize--;
                  value = -value;
               }

               unsigned char digitBuffer[BIT_SIZEOF(value)+1];
               unsigned char* revPtr = digitBuffer + ARRAY_SIZE(digitBuffer);
               *--revPtr = '\0';
               assert( base > 0 );
               do
               {
                  unsigned char c = (value % base) + '0';
                  if( c > '9' )
                     c += hexOffset;
                  *--revPtr = c;
                  value /= base;
                  assert( revPtr >= digitBuffer );
                  if( paddingSize > 0 )
                     paddingSize--;
               }
               while( value > 0 );

               while( (paddingSize-- != 0) && (revPtr > digitBuffer) )
                  *--revPtr = paddingChar;

               rOutput += reinterpret_cast<char*>(revPtr);

               FSM_TRANSITION( NORMAL, label='param was read', color=blue );
            } /* case PARAM */
         } /* switch( state ) */
      }
      while( next );
   } /* for( uint i = 0; i < format.length(); i++ ) */
}

//================================== EOF ======================================
