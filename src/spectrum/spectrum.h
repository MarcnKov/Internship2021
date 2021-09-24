/**
 * @file     spectrum.h
 * @author   IGOSat Software Team - SCI intern 2020 : Sanath Muret
 *           IGOSat Scintillator Team - SCI intern 2021 : Marcin Kovalevskij 
 * @brief    Easiroc spectrum mode implementation for the Scintillator payload
 * @version  2.0
 * @date     2021-07-06
 * 
 */

#ifndef     _SPECTRUM_H_
#define     _SPECTRUM_H_

#include    <stdio.h>
#include    <stdlib.h>
#include    <math.h>

//------------------------------------------------------------------------------
//         MARCO DEFINITION
//------------------------------------------------------------------------------

#define     SPECTRUM__C_SIGNAL_VALID                          (1)
/**
 * Number of hardware, EASIROC output channels.
 * Only first 26 channels contain information
 */
#define     SCI_COMMON__C_NUMBER_OF_CHANNEL                  (32)
/**
 * Number of channels that are used for particle detection
 */   
#define     SCI_COMMON__C_NUMBER_OF_SIPM                     (26)
/**
 * Total number of data acquisitions
 */
#define     SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE  (1000)
/**
 * Total number of data acquisitions for light curves
 */
#define     SPECTRUM__C_MAX_BIN_NUMBER_LIGHT_CURVE         (1000)
/**
 * Max array lenght of arrays stored in a
 * SPECTRUM_T_LC_PER_SP_TIME struct
 */
#define     SP__C_MAX_LC_PER_EN_ARR_LEN                    (1000)
/**
 * Number of energies for which to make separate light curves
 */
#define     SPECTRUM_PARAM_LC_per_SP_arr_size                (5)
/**
 * Number of energy bins for construction of spectrum histograms
 */
#ifdef SPECTRUM_binning
    #define SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM (512) //TO DO : DEFINE AS PARAMETER/GLOBAL VARIABLE 
#else
    #define SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM (1024)
#endif
/**
 * Inorganic scintillator pin correspondance with the EASIROC microcontroller.
 * Detection of photons and electrons.
 */
//For Na22 source data
// #define     SPECTRUM__C_CEBR3_SIPM          (0b00000000110000000000000000)
//For Co60 source data
#define     SPECTRUM__C_CEBR3_SIPM          (0b00000000000000000000000000)
/**        
 * Organic scintillator pin correspondance with the EASIROC microcontroller.
 * Detection of electrons.
 */
//For Na22 source data
// #define     SPECTRUM__C_EJ200_SIPM          (0b00000000001111111111111111)
#define     SPECTRUM__C_EJ200_SIPM          (0b00000000000000000000000110)

//------------------------------------------------------------------------------
//         SOURCE_TYPE & ADC_GAIN enum
//------------------------------------------------------------------------------

/**
 * @brief The type of energy source :
 * 0 : gamma photon,
 * 1 : electron,
 * 2 : undefined
 * 
 */
typedef enum _SPECTRUM__E_SOURCE_TYPE
{
    SOURCE_TYPE_PHOTON = 0,
    SOURCE_TYPE_ELECTRON,
    SOURCE_TYPE_UNDEFINED

}SPECTRUM__E_SOURCE_TYPE;
/**
 * @brief The ADC gain type : High gain or low gain
 * 
 */
typedef enum _SPECTRUM__E_ADC_GAIN
{
    ADC_LG = 0,
    ADC_HG
    
}SPECTRUM__E_ADC_GAIN;
//------------------------------------------------------------------------------
//         STRUCTURES
//------------------------------------------------------------------------------

/**
 * @brief One channel energy struct that store the id of the channel and the 
 *        associated energy converted from the corresponding channel ADC value
 *        and  corrected with a gain and an offset
 * 
 */
typedef struct _SPECTRUM__T_ONE_CHANNEL_ENERGY
{
	unsigned short    id; /*!< id of the channel */
	unsigned short value; /*!< converted and corrected value of the measured energy */

}SPECTRUM__T_ONE_CHANNEL_ENERGY;
/**
 * @brief    One acquisition period treated data struct that store the type of 
 *           source for the corresponding acquisition and the sum of every channel
 *           energy
 *           
 * @warning  float date should be changed to unsigned int type
 * 
 */
typedef struct _SPECTRUM__T_ONE_MEASUREMENT
{
	SPECTRUM__E_SOURCE_TYPE source_type; /*!< type of source measured */
	unsigned int sum;                    /*!< Value of the sum of every channel*/
    float        date;                   /*!< Date of acquisition of the measurement*/

}SPECTRUM__T_ONE_MEASUREMENT;
/**
 * @brief   ADC value struct definition, for each channel of LG or HG, 
 * 		    we create a ADC struct value which correspond to the aquired 
 *          value from the ADC in both HG and LG for a given channel id
*/
typedef struct _SPECTRUM__T_ONE_CHANNEL_ADC_VALUE
{   
	unsigned short       id; /*!< id of the channel */
    unsigned short HG_value; /*!< High gain value of a measurement */
	unsigned short LG_value; /*!< Low  gain value of a measurement */

}SPECTRUM__T_ONE_CHANNEL_ADC_VALUE;
/**
 * @brief   Struct definition of all ADC channel values. The struct
 *          stores the value of each channel during the whole 
 *          aquisition process.
 *   
 */
typedef struct _SPECTRUM_T_ALL_CHANNEL_ADC_VALUE
{	
    /* Array of all 32 channels values corresponding to one period of acquisition.*/
    SPECTRUM__T_ONE_CHANNEL_ADC_VALUE one_period_value[SCI_COMMON__C_NUMBER_OF_CHANNEL];
    float date; /*!< Start date of the acquisition period.*/

}SPECTRUM__T_ALL_CHANNEL_ADC_VALUE;
/**
 * @brief   Contains  energy min & max bounds for
 *          each particle type. Energy bounds are
 *          used to construct SPECTRUM histograms.                              
 * 
 */
typedef struct _SPECTRUM__T_SPECTRUM_ENERGY_BOUNDS
{
    unsigned int ph_min_bound, ph_max_bound; 
    unsigned int el_min_bound, el_max_bound;
    unsigned int un_min_bound, un_max_bound;

}SPECTRUM__T_SPECTRUM_ENERGY_BOUNDS;
/**
 * @brief   Data structure that contains spectrum
 *          histograms for each particle type.
 *     
 */
typedef struct SPECTRUM__T_SPECTRUM_TYPE
{
    unsigned int photon_axis   [SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM];
    unsigned int electron_axis [SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM];
    unsigned int undefined_axis[SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM];

}SPECTRUM__T_SPECTRUM_TYPE;
/**
 * @brief Contains spectrum histograms output data.
 *   
 */
typedef struct SPECTRUM__T_SPECTRUM_MEASUREMENT_OUTPUT
{  
    SPECTRUM__T_SPECTRUM_TYPE spectrum_particle_axis;
    float  date; /*!< Data tratment date.*/

}SPECTRUM__T_SPECTRUM_MEASUREMENT_OUTPUT;
/**
 * @brief Contains counters for three particle types.
 *        Used to construct LC (Light Curves).
 */
typedef struct SPECTRUM__T_particle_counts
{
    unsigned int photon_counts;
    unsigned int electron_counts;
    unsigned int undefined_counts;

}SPECTRUM_T_particle_counts;
/**
 * @brief Data structure that contains LC for each particle type.   
 * 
 */
typedef struct SPECTRUM__T_LIGHT_TYPE
{
    unsigned int    photon_axis[SPECTRUM__C_MAX_BIN_NUMBER_LIGHT_CURVE];
    unsigned int  electron_axis[SPECTRUM__C_MAX_BIN_NUMBER_LIGHT_CURVE]; 
    unsigned int undefined_axis[SPECTRUM__C_MAX_BIN_NUMBER_LIGHT_CURVE];

}SPECTRUM__T_LIGHT_TYPE;
/**
 * @brief Contains LC output data.
 *   
 */
typedef struct SPECTRUM__T_LIGHT_MEASUREMENT_OUTPUT
{  
    SPECTRUM__T_LIGHT_TYPE light_particle_axis;
    unsigned int SPECTRUM_G_CNST_lc_step; /*!< LC time step, is necessary for construction of LC plots.*/
    float last_time_step; /*!< Last time step, is necessary for construction of LC plots.*/   

}SPECTRUM__T_LIGHT_MEASUREMENT_OUTPUT;


typedef struct SPECTRUM__T_LC_PER_SP_TIME
{   

    unsigned short sparse_array[SP__C_MAX_LC_PER_EN_ARR_LEN];

    float time_array[SP__C_MAX_LC_PER_EN_ARR_LEN];
    float time_step;
    
    unsigned int num_part_per_dt[SP__C_MAX_LC_PER_EN_ARR_LEN];
    unsigned int index_shift;
    unsigned int max_arr_size;
    unsigned int sparse_array_len;
    unsigned int struct_num_consec_zeros;

}SPECTRUM_T_LC_PER_SP_TIME;


typedef struct SPECTRUM__T_LC_PER_SP_ENERGY
{
    SPECTRUM_T_LC_PER_SP_TIME ph_el_un_arr[3];

}SPECTRUM_T_LC_PER_SP_ENERGY;

//------------------------------------------------------------------------------
//         Functions definition
//------------------------------------------------------------------------------

/**
 * @brief   Function to get acces to the first element of the array
 *          READING_G_acquisition_period to use this array in external module. 
 * 
 * @return  READING__T_ALL_CHANNEL_ADC_VALUE*   The adress of the first array field.
 */
SPECTRUM__T_ALL_CHANNEL_ADC_VALUE* SPECTRUM_get_acquisition_period_array(void);
/**
 * @brief Setup function to define coefficients of the data processing part :
 *        the low threshold, the high threshold, the offset, the gain and the 
 *        HG threshold
 * 
 * @param coeff_array The array of values we want to assign to a coefficients 
 *                    array
 * 
 * @param global_coeff_array The global coefficient array to be assigned
 * 
 * @param size The size of the arrays we treat
 */
void SPECTRUM_coeff_definition(const unsigned int *coeff_array, unsigned int *global_coeff_array, int size);
/**
 * @brief Function that determine if a signal is valid or not. When the signal
 *        is valid, we set the channel-corresponding bit in a long. This long
 *        correspond only to one type of signal gain (HG or LG)
 * 
 * @param acquisition the aquisition array corresponding to a the acquisition of
 *                    all 32 channels
 * 
 * @param gain_type Type of signal we want to analyse (Hg or LG) of the aquisition
 * 
 * @return unsigned long The validity ulong which report each channel signal 
 *                       validity
 */
unsigned long SPECTRUM_signal_validity(SPECTRUM__T_ALL_CHANNEL_ADC_VALUE acquisition, SPECTRUM__E_ADC_GAIN gain_type);
/**
 * @brief Function to count how many valid signal we have for a given validity 
 *        ulong
 * 
 * @param validity The validity ulong which report each channel signal validity
 * 
 * @return unsigned int the number of valid signal
 */
unsigned int SPECTRUM_valid_signal_counter( unsigned long validity );
/**
 * @brief Function to calculate the corrected value of the energy. First it
 *        convert from the ADC value than it correct with the gain and the 
 *        offset given during the setup
 * 
 * @param adc_value The ADC value to convert and correct
 * 
 * @param index The index of the signal to correct
 * 
 * @return unsigned int The corrected value of the signal
 */
unsigned int SPECTRUM_energy( unsigned int adc_value, int index, SPECTRUM__E_ADC_GAIN gain_type);
/**
 * @brief The function that correct the acquisitions signal into a corrected
 *        energy and write them into a given array of ENERGY 
 * 
 * @param energy_array The array of energy values we write in and modify
 * 
 * @param acquisition The Acquisition period values to threat
 * 
 * @param validity The validity ulong which report each channel signal validity
 * 
 * @param gain_type The type of signal gain we treat
 */
void SPECTRUM_signal_correction(SPECTRUM__T_ONE_CHANNEL_ENERGY energy_array[SCI_COMMON__C_NUMBER_OF_CHANNEL], 
                                SPECTRUM__T_ALL_CHANNEL_ADC_VALUE acquisition, 
                                unsigned long validity, 
                                SPECTRUM__E_ADC_GAIN gain_type);
/**
 * @brief Function that determine if the source is a gamma photon, an electron 
 *        or even something we dont know
 * 
 * @param validity The validity ulong which report each channel signal validity
 * 
 * @return SPECTRUM__E_SOURCE_TYPE The type of source among the following list :
 *                                 SIGNAL_GAMMA, SIGNAL_ELCTRON, SIGNAL_ONSP
 */
SPECTRUM__E_SOURCE_TYPE SPECTRUM_source_definition( unsigned long validity );
/**
 * @brief Function that sum energies according to the defined rules depending
 *        the source type
 * 
 * @param energy_array The array of energies we want to sum
 * 
 * @param validity The validity ulong which report each channel signal validity
 * 
 * @param source_type The type of studied source
 * 
 * @return unsigned intprocess The sum results
 */
unsigned int SPECTRUM_signal_sum(const SPECTRUM__T_ONE_CHANNEL_ENERGY energy_array[SCI_COMMON__C_NUMBER_OF_CHANNEL], 
                                 unsigned long validity,
                                 SPECTRUM__E_SOURCE_TYPE source_type);
/**
 * @brief The general function of the spectrum module that treat one full 
 *        READING_G_acquisition_period array and output signal data.
 * 
 */
int  SPECTRUM_data_treatement(void);
/**
 * @brief   prepares the compressed data of light curves per energy for
 *          each particle type
 * 
 */
void SP_LC_per_energy_generation( void );
/**
 * @brief   support function for the spectrum algorithm,
            calculates the energy differences
 *          
 * 
 */
unsigned int* SP_generation_helper(void);

/**
 * @brief      calculates minimum and maximum energies for each particle type
 */
void SPECTRUM_energy_min_max(SPECTRUM__E_SOURCE_TYPE source_type);
/**
 * @brief       verifies if energy spectrum index is within the
                predefined array size
 */
unsigned int check_spectrum_index(unsigned int index_spectrum);
/**
 * @brief       updates the SPECTRUM_T_LC_PER_SP_ENERGY data strucutre
 *              for each particle by counting how many particles,
 *              of a given energy, weren't detected in a given time step
 *               
 */
void SP_classify_energies(  unsigned int i,
                            unsigned int index_sp,
                            SPECTRUM__E_SOURCE_TYPE src_type);
/**
 * @brief       calculates light curves for each particle type and energy type 
                
 */
void SP_LC_per_energy_time_regrouping(  SPECTRUM_T_LC_PER_SP_TIME *part_type,
                                        SPECTRUM__E_SOURCE_TYPE source_type);
/**
 * @brief      Spectrum generation with all measurement array
 */
int SPECTRUM_spectrum_generation (void);
/**
 * @brief  SPECTRUM module driver function.  
 *           
 * @return 1 on success, else 0
 */
int SPECTRUM_process(void); 
/**
 * @brief      Updates LC plots output array.
 *
 * @param      particle_counts  The particle counts contains number of particles per given time step.
 */
void SPECTRUM_update_output_array(SPECTRUM_T_particle_counts *particle_counts);
/**
 * @brief Counts number of number of particles detected per given time step
 *        for a given particle. 
 *       
 * @param                i  Index to iterate over energy sums of all signals. 
 * @param  particle_counts  The particle counts contains number of particles per given time step.
 * @param         sum_flag  If the sum flag = 1, increment number of particles detected per given time step
 *                          for a given particle.
 */
void SPECTRUM_update_particle_axis(unsigned int i, SPECTRUM_T_particle_counts *particle_counts, unsigned short sum_flag);
/**
 * @brief      Generates LC. Calculates number of particle incidents, and their type, during a given time step.
 */
void SPECTRUM_light_curve_generation(void);
/**
 * @brief      Prints a particle detection summary.
 */
void print_particle_detection_summary(void);
/**
 * @brief      Prints a particle detection summary
               for the energies specified in
               PARAM_LC_per_SP_ENERGY_values array. 
 */
void print_lc_per_energy_detection_summary(void);
/**
 * @brief   get array containing energies for light curves
            per energy
 * 
 */
void INPUTOUTPUT_write_lc_per_energy_to_file( void );
/**
 * @brief Offset array getter
 * 
 */
unsigned int*  SPECTRUM_get_offset_array( void );
/**
 * @brief Gain array getter
 * 
 */
unsigned int* SPECTRUM_get_gain_array( void );
/**
 * @brief HG threshold getter
 * 
 */
unsigned int* SPECTRUM_get_threshold_hg( void );
/**
 *
 * @brief High threshold getter
 * 
 */
unsigned int* SPECTRUM_get_high_threshold( void );
/**
 * @brief Low threshold getter
 * 
 */
unsigned int* SPECTRUM_get_low_threshold( void );
/**
 * @brief Energy min bound getter
 * 
 */
unsigned int* SPECTRUM_get_energy_min_bound( void );
/**
 * @brief Energy max bound getter
 * 
 */
unsigned int* SPECTRUM_get_energy_max_bound( void );
/**
 * @brief Energy step getter
 * 
 */
unsigned int* SPECTRUM_get_energy_step( void );
/**
 * @brief Light Curve array size getter
 * 
 */
unsigned int* SPECTRUM_get_lc_array_size(void);
/**
 * @brief     Spectrums histograms photon number of bin getter.
 *
 */
unsigned int* SPECTRUM_get_number_of_bin_ph(void);
/**
 * @brief     Spectrums histograms electron number of bin getter.
 *
 */
unsigned int* SPECTRUM_get_number_of_bin_el(void);
/**
 * @brief     Spectrums histograms undefined number of bin getter.
 *
 */
unsigned int* SPECTRUM_get_number_of_bin_un(void);
/**
 * @brief      SPECTRUM LC time step getter.
 *
 */
unsigned int* SPECTRUM_get_G_CNST_lc_step(void);
/**
 * @brief   get array containing energies for light curves
 *          per energy
 *
 */
unsigned short* SP_get_energies_per_LC( void );
/**
 * @brief      Energy bounds (min, max) getter for each
 *             particle type.
 *             
 */
SPECTRUM__T_SPECTRUM_ENERGY_BOUNDS* SPECTRUM_G_get_energy_bounds(void);
/**
 * @brief      SPECTRUM__T_ALL_CHANNEL_ADC_VALUE struct for all measurements getter, to use it in external module.    
 *
 */
SPECTRUM__T_ALL_CHANNEL_ADC_VALUE* SPECTRUM_G_get_acquisition_period( void );
/**
 * @brief      SPECTRUM__T_SPECTRUM_MEASUREMENT_OUTPUT getter, to use it in external module.  
 *
 */
SPECTRUM__T_SPECTRUM_MEASUREMENT_OUTPUT* SPECTRUM_G_get_spectrum_measurement_output(void);
/**
 * @brief      SPECTRUM__T_LIGHT_MEASUREMENT_OUTPUT getter, to use it in external module. 
 *
 */
SPECTRUM__T_LIGHT_MEASUREMENT_OUTPUT* SPECTRUM_G_get_light_measurement_output( void );
/**s
 * @brief      SPECTRUM__T_LIGHT_MEASUREMENT_OUTPUT getter, to use it in external module. 
 *
 */
SPECTRUM_T_LC_PER_SP_ENERGY* SP_get_lc_per_energy_array(void);

#endif
