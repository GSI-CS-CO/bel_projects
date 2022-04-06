/*!
 * @file  scu_control_config.h
 * @brief Definition of compiler switches. E.g.: CONFIG_XXX
 *
 * @note Header only.
 *
 * @note This file shall be included in all source files which depend
 *       on each other.
 * @date 14.10.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 */
#ifndef _SCU_CONTROL_CONFIG_H
#define _SCU_CONTROL_CONFIG_H

/*!
 * @todo Remove this switch asap!!!
 */
#define _CONFIG_PATCH_PHASE

/*!
 * @brief If defined then the maximum and minimum of all whishbone accesses
 *        becomes printed in stderr.
 */
#define CONFIG_EB_TIME_MEASSUREMENT

/*!
 * @brief MIL-DAQ-buffer-handlong is backward compatible to the old
 *        LM32-firmware
 */
#define CONFIG_MILDAQ_BACKWARD_COMPATIBLE

#define _CONFIG_WAS_READ_FOR_ADDAC_DAQ

#ifdef __cplusplus
#if 1
  #ifndef _BSD_SOURCE
    #define _BSD_SOURCE
  #endif
  #ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE
  #endif
#endif
  #ifndef _DEFAULT_SOURCE
    #define _DEFAULT_SOURCE
  #endif
  #if (_GLIBCXX_USE_CXX11_ABI != 0) && (__GNUC__ < 5)
    #undef _GLIBCXX_USE_CXX11_ABI
    /*!
     * @brief Necessary for backward compatibility if the compiler
     *        version greater or equal 5
     */
    #define _GLIBCXX_USE_CXX11_ABI 0
  #endif
  #ifndef BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
    /*!
     * @brief Necessary for backward compatibility if the compiler
     *        version greater or equal 5
     */
     #define BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
  #endif

#endif /* ifdef __cplusplus */

#ifndef CONFIG_GSI
 /*!
  * @brief Is a GSI project.
  */
 #define CONFIG_GSI
#endif
#ifndef CONFIG_GSI_FE
 /*!
  * @brief Is a project of the ACO-group front-end.
  */
 #define CONFIG_GSI_FE
#endif

/*!
 * @brief We still use SCU 3.
 */
#define CONFIG_SCU 3

/*!
 * @brief Providing LM32 firmware major version 3
 */
//#define CONFIG_FW_VERSION_3

/*!
 * @brief MIL function generators will used.
 */
#define CONFIG_MIL_FG

/*!
 * @brief ADDAC/ACU DAQS in LM32 Firmware integrated.
 */
#define CONFIG_SCU_DAQ_INTEGRATION

/*!
 * @brief Maximum ADDAC/ACU DAQ-channels per SCU-bus slave.
 */
#define DAQ_MAX_CHANNELS 4

/*!
 * @brief Using DDR3 memory on SCU
 */
#if (CONFIG_SCU == 3) && !defined( CONFIG_SCU_USE_DDR3 )
 #define CONFIG_SCU_USE_DDR3
#endif

#if defined( CONFIG_SCU_USE_DDR3 ) && (CONFIG_SCU != 3 )
  #error CONFIG_SCU_USE_DDR3 can only defined on SCU3!
#endif

/*!
 * @brief The DDR3 burst functions will not used.
 */
#define CONFIG_DDR3_NO_BURST_FUNCTIONS

#endif /* ifndef _SCU_CONTROL_CONFIG_H */
/*================================== EOF ====================================*/
