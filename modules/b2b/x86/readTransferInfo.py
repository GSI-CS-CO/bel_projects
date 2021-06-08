#!/usr/bin/env python
# encoding: utf-8

import requests
import sys
from argparse import ArgumentParser

LSA_SETTINGS_REST_ENDPOINT = 'https://restpro00a.acc.gsi.de/lsa/client/v2/settings'
BEAM_PROCESS_PURPOSE_EXT = 'EXTRACTION_FAST'
BEAM_PROCESS_PURPOSE_INJ = 'RING_INJECTION'
PARAMETERFREQ_NAME = 'BEAM/FREV'
PARAMETERSEQ_NAME = 'TIMEPARAM/TIMING_SEQUENCE_KEY'


def print_available_patterns():
    response = requests.get(LSA_SETTINGS_REST_ENDPOINT)
    print("\n" + "available Patterns:")
    for pattern_name in response.json():
        print('  ' + pattern_name)  


def main():
    parser = ArgumentParser(description='Really simple script that reads the value of the parameters "PARAMETER..." '
                            'from the Beam Process that carries "EXTRACTION_FAST" or "RING_INJECTION" within its name'
                            'for a Pattern that the user specifies from the PRO database')
    parser.add_argument(dest='ring_name', help='name of the Ring to read value for ("SIS18", "ESR")',
                        metavar='ring-name', nargs='?')
    parser.add_argument(dest='pattern_name', help='name of the Pattern to read value for',
                        metavar='pattern-name', nargs='?')


    args         = parser.parse_args()
    ring_name    = args.ring_name
    pattern_name = args.pattern_name


    if ring_name == None:
        parser.print_help()
        exit()
    if pattern_name == None:
        parser.print_help()
        print_available_patterns()
    else:

        parameter_ring_freq_name = ring_name+PARAMETERFREQ_NAME
        parameter_ring_seq_name  = ring_name+PARAMETERSEQ_NAME

        chains_in_pattern = requests.get(LSA_SETTINGS_REST_ENDPOINT + '/' + pattern_name).json()
        chain_name = chains_in_pattern[0]
        beam_processes_in_chain = requests.get(LSA_SETTINGS_REST_ENDPOINT + '/' + pattern_name + '/' + 
                                               chain_name).json()
        injection_beam_process_name = [beam_processes for beam_processes in beam_processes_in_chain 
                                        if BEAM_PROCESS_PURPOSE_INJ in beam_processes][0]
        injection_frequency_value = requests.get(LSA_SETTINGS_REST_ENDPOINT + '/' + pattern_name + '/' + chain_name + 
                                                  '/' + injection_beam_process_name + '/' + 
                                                  parameter_ring_freq_name).json()['value']
        injection_sequence_index = requests.get(LSA_SETTINGS_REST_ENDPOINT + '/' + pattern_name + '/' + chain_name +

                                                 '/' + injection_beam_process_name + '/' + parameter_ring_seq_name).json()['value'] 


        chains_in_pattern = requests.get(LSA_SETTINGS_REST_ENDPOINT + '/' + pattern_name).json()
        chain_name = chains_in_pattern[0]
        beam_processes_in_chain = requests.get(LSA_SETTINGS_REST_ENDPOINT + '/' + pattern_name + '/' + 
                                               chain_name).json()
        extraction_beam_process_name = [beam_processes for beam_processes in beam_processes_in_chain 
                                        if BEAM_PROCESS_PURPOSE_EXT in beam_processes][0]
        extraction_frequency_value = requests.get(LSA_SETTINGS_REST_ENDPOINT + '/' + pattern_name + '/' + chain_name + 
                                                  '/' + extraction_beam_process_name + '/' + 
                                                  parameter_ring_freq_name).json()['value']
        extraction_sequence_index = requests.get(LSA_SETTINGS_REST_ENDPOINT + '/' + pattern_name + '/' + chain_name +
                                                 '/' + extraction_beam_process_name + '/' + parameter_ring_seq_name).json()['value'] 

        print('injnue  :', injection_frequency_value)
        print('injsid  :', injection_sequence_index)
        print('extnue  :', extraction_frequency_value)
        print('extsid  :', extraction_sequence_index)
       

if __name__ == "__main__":
    sys.exit(main())

