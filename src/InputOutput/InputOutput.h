/**
 * @file    InputOutput_handling.h
 * @author  IGOSat Scintillator Team - SCI intern 2021 : Marcin Kovalevskij
 * @brief   Easiroc data acquisition implementation for the Scintillator payload
 * @version 0.1
 * @date    2021-07-06
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _INPUTOUTPUT_H_
#define _INPUTOUTPUT_H_

#include "../spectrum/spectrum.h"

/**
 * @brief   Function to read test data of HG and LG values
 *          from a .txt file
 * 
 * @param   start_index - start_index of an acquisition
 * 
 * @param   end_index - end_index of an acquisition
 * 
 * @return  void
 */
void INPUTOUTPUT_read_test_data_from_file(unsigned int start_index, unsigned int end_index);
/**
 * @brief   Function to read test data of HG and LG values
 *          from an array
 * 
 * @param   start_index - start_index of an acquisition
 * 
 * @param   end_index - end_index of an acquisition
 * 
 * @return  void
 */
void INPUTOUTPUT_read_test_data_from_array(unsigned int start_index, unsigned int end_index);
/**
 * @brief      Function to write analyzed LC test data to a file.
 *
 * @return     void
 */
void INPUTOUTPUT_write_LC_to_file(void);
/**
 * @brief      Function to write analyzed SP test data to a file.
 * 
 * @return     void
 */
void INPUTOUTPUT_write_SP_to_file(void);

#endif

