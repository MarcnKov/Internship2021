#include "../InputOutput/InputOutput.h"
//------------------------------------------------------------------------------
//         STRUCT initialization 
//------------------------------------------------------------------------------

static SPECTRUM__T_ALL_CHANNEL_ADC_VALUE    SPECTRUM_G_acquisition_period[SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE] = {0};
static SPECTRUM__T_ONE_MEASUREMENT          SPECTRUM_all_measurements[SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE]     = {0};

static SPECTRUM__T_SPECTRUM_MEASUREMENT_OUTPUT  G_ONE_SPECTRUM_MEASUREMENT_OUTPUT                                        = {0};
static SPECTRUM__T_LIGHT_MEASUREMENT_OUTPUT     G_ONE_LIGHT_MEASUREMENT_OUTPUT                                           = {0};
static SPECTRUM__T_SPECTRUM_ENERGY_BOUNDS       G_ENERGY_BOUNDS                                                          = {0};

static SPECTRUM_T_LC_PER_SP_ENERGY G_LC_PER_SP_energy[SPECTRUM_PARAM_LC_per_SP_arr_size]                                 = {0};

//------------------------------------------------------------------------------
//         Variable definition 
//------------------------------------------------------------------------------

static unsigned int SPECTRUM_G_number_bin_ph                                    =     0;
static unsigned int SPECTRUM_G_number_bin_el                                    =     0;
static unsigned int SPECTRUM_G_number_bin_un                                    =     0;
            
static unsigned int SPECTRUM_G_PARAM_number_bin_lc                              =    20; //TO MODIFY IN ORDER TO SCALE LC

static unsigned int SPECTRUM_G_PARAM_spectrum_energy_step                       =    100; //TO MODIFY IN ORDER TO SCALE SP HISTOGRAM

static unsigned int SPECTRUM_G_PARAM_threshold_hg                               = 25000;

static unsigned int SPECTRUM_G_PARAM_high_thresh_HG                             =  4095; // HG & LG high and high thresholds are the same
static unsigned int SPECTRUM_G_PARAM_high_thresh_LG                             =  4095;

static unsigned int SPECTRUM_G_PARAM_low_thresh_HG                              =   650; // HG & LG high and low thresholds are different
static unsigned int SPECTRUM_G_PARAM_low_thresh_LG                              =   650; 

static unsigned int SPECTRUM_G_PARAM_offset[SCI_COMMON__C_NUMBER_OF_CHANNEL]    =   {0}; //signal correction
static unsigned int SPECTRUM_G_PARAM_gain[SCI_COMMON__C_NUMBER_OF_CHANNEL]      =   {0}; //TO DEFINE GAIN ARRAY
//While changing the nubmer of energies, you shuld also to modify SPECTRUM_PARAM_LC_per_SP_arr_size variable
static unsigned short PARAM_LC_per_SP_ENERGY_values[]                           = {7,8,9,10,11};

//VARIABLES BELOW SHOULDN'T BE MODIFIED
static unsigned int SPECTRUM_G_CNST_lc_array_size                               =     0; //counter
static unsigned int SPECTRUM_G_CNST_lc_step                                     =     0;
const  unsigned int SP_G_CNST_acq_duration                                      =  1000;

//------------------------------------------------------------------------------
//         Function definition
//------------------------------------------------------------------------------

void SPECTRUM_coeff_definition( const unsigned int* coeff_array, unsigned int* global_coeff_array,
                                int size )
{
    int i = 0;
    for(i = 0; i < size; i++)
    {
        *(global_coeff_array++) = *(coeff_array++);
    }
}

unsigned long SPECTRUM_signal_validity( SPECTRUM__T_ALL_CHANNEL_ADC_VALUE acquisition,
                                        SPECTRUM__E_ADC_GAIN gain_type )
{
    unsigned long signal_validity = 0;
    int           i               = 0;

    if (gain_type == ADC_HG)
    {
        for (i = 0; i < SCI_COMMON__C_NUMBER_OF_CHANNEL; i++ ) 
        {
            if (acquisition.one_period_value[i].HG_value >= SPECTRUM_G_PARAM_low_thresh_HG && 
                acquisition.one_period_value[i].HG_value <= SPECTRUM_G_PARAM_high_thresh_HG)
            {
                signal_validity |= (SPECTRUM__C_SIGNAL_VALID<<i);
            }
        }
    }
    else
    {
        for (i = 0; i < SCI_COMMON__C_NUMBER_OF_CHANNEL; i++ ) 
        {
            if (acquisition.one_period_value[i].LG_value > SPECTRUM_G_PARAM_low_thresh_LG && 
                acquisition.one_period_value[i].LG_value < SPECTRUM_G_PARAM_high_thresh_LG)
            {
                signal_validity |= (SPECTRUM__C_SIGNAL_VALID<<i);
            }
        }
    }
    return signal_validity;
}

unsigned int SPECTRUM_energy(unsigned int adc_value, int index, SPECTRUM__E_ADC_GAIN gain_type)
{
    unsigned int    signal       = 0;
    unsigned int    energy_value = 0;

    signal = adc_value;

    if (gain_type == ADC_HG)
    {
        energy_value = (SPECTRUM_G_PARAM_gain[index]) * (signal-SPECTRUM_G_PARAM_low_thresh_HG) +
        SPECTRUM_G_PARAM_offset[index];
    }
    else
    {
        energy_value = (SPECTRUM_G_PARAM_gain[index]) * (signal-SPECTRUM_G_PARAM_low_thresh_LG) +
        SPECTRUM_G_PARAM_offset[index];   
    }

    return energy_value;
}


void SPECTRUM_signal_correction(SPECTRUM__T_ONE_CHANNEL_ENERGY energy_array[SCI_COMMON__C_NUMBER_OF_CHANNEL], 
                                SPECTRUM__T_ALL_CHANNEL_ADC_VALUE acquisition, unsigned long validity, 
                                SPECTRUM__E_ADC_GAIN gain_type)
{
    unsigned int i = 0;
    for (i = 0; i < SCI_COMMON__C_NUMBER_OF_CHANNEL; i++) 
    {   
        energy_array[i].id = acquisition.one_period_value[i].id;

        if  (((1 << 1) - 1) & (validity >> i))
        {
            if (gain_type == ADC_HG)
            {
                energy_array[i].value = SPECTRUM_energy(acquisition.one_period_value[i].HG_value, i, ADC_HG);
            }
            else
            {
                energy_array[i].value = SPECTRUM_energy(acquisition.one_period_value[i].LG_value, i, ADC_LG);
            }
        }
    }
}


unsigned int SPECTRUM_valid_signal_counter( unsigned long validity )
{
    unsigned int set_bit_count = 0; 
    while (validity) 
    { 
        validity &= (validity - 1);
        set_bit_count++; 
    }
    return set_bit_count;
}

SPECTRUM__E_SOURCE_TYPE SPECTRUM_source_definition( unsigned long validity )
{
    unsigned int counter_cebr3 = 0; 
    unsigned int counter_ej200 = 0;

    counter_cebr3 = SPECTRUM_valid_signal_counter (validity & SPECTRUM__C_CEBR3_SIPM);
    counter_ej200 = SPECTRUM_valid_signal_counter (validity & SPECTRUM__C_EJ200_SIPM);

    if ((counter_cebr3 + counter_ej200) >= 18)
    {
        return SOURCE_TYPE_ELECTRON;
    }
    else if (counter_cebr3 == 16)
    {
        return SOURCE_TYPE_PHOTON;
    }
    else 
    {
        return SOURCE_TYPE_UNDEFINED;
    }
}


unsigned int SPECTRUM_signal_sum(   const SPECTRUM__T_ONE_CHANNEL_ENERGY energy_array[SCI_COMMON__C_NUMBER_OF_CHANNEL], 
                                    unsigned long validity,
                                    SPECTRUM__E_SOURCE_TYPE source_type )
{
    unsigned int sum = 0;
    unsigned int i;

    if (source_type == SOURCE_TYPE_PHOTON) 
    {
        for (i = 0; i < SCI_COMMON__C_NUMBER_OF_CHANNEL; i++) 
        {
            if (((1 << 1) - 1) & (SPECTRUM__C_CEBR3_SIPM >> i))
            {
                sum += energy_array[i].value;
            }
        }
    }
    else 
    {
        for (i = 0; i < SCI_COMMON__C_NUMBER_OF_CHANNEL; i++) 
        {
            if (((1 << 1) - 1) & (validity >> i))
            {
                sum += energy_array[i].value;
            }
        }
    }
    return sum;
}

int  SPECTRUM_data_treatement( void )
{
    unsigned int                        sum_hg              = 0;
    unsigned int                        sum_lg              = 0;
    unsigned int                             i              = 0;
    unsigned long                       valid_hg            = 0;
    unsigned long                       valid_lg            = 0;
    
    SPECTRUM__E_SOURCE_TYPE             source_type_hg      = 0;
    SPECTRUM__E_SOURCE_TYPE             source_type_lg      = 0;

    SPECTRUM__T_ONE_CHANNEL_ENERGY      energy_array_hg[SCI_COMMON__C_NUMBER_OF_CHANNEL]  = {0};
    SPECTRUM__T_ONE_CHANNEL_ENERGY      energy_array_lg[SCI_COMMON__C_NUMBER_OF_CHANNEL]  = {0};

    for (i = 0; i < SCI_COMMON__C_NUMBER_OF_CHANNEL; i++)
    {
        SPECTRUM_G_PARAM_gain[i] = 1;
    }

    const SPECTRUM__T_ALL_CHANNEL_ADC_VALUE*   SPECTRUM_G_acquisition_period_ptr = 
    SPECTRUM_get_acquisition_period_array();

    for (i = 0; i < SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE; i++)
    {
        // Signal validity checking
        valid_hg = SPECTRUM_signal_validity(*(SPECTRUM_G_acquisition_period_ptr+i), ADC_HG);
        valid_lg = SPECTRUM_signal_validity(*(SPECTRUM_G_acquisition_period_ptr+i), ADC_LG);

        //if (temps valide) : continue, else message d'erreur

        // Correction of valid signal
        SPECTRUM_signal_correction( energy_array_hg, 
            *(SPECTRUM_G_acquisition_period_ptr+i), 
            valid_hg, 
            ADC_HG);    

        SPECTRUM_signal_correction( energy_array_lg, 
            *(SPECTRUM_G_acquisition_period_ptr+i), 
            valid_hg, 
            ADC_LG);
        
        // Source identification according to valid signal
        source_type_hg = SPECTRUM_source_definition (valid_hg);
        source_type_lg = SPECTRUM_source_definition (valid_lg);
        
        sum_hg = SPECTRUM_signal_sum(energy_array_hg, valid_hg, source_type_hg);
        sum_lg = SPECTRUM_signal_sum(energy_array_lg, valid_lg, source_type_lg);
        
        //verify for varialbe overflow
        sum_hg = sum_hg > 0xFFFF ? 0xFFFF : sum_hg;
        sum_lg = sum_lg > 0xFFFF ? 0xFFFF : sum_lg;

        // Depending the sum value, we keep the correct value (HG or LG)
        if (sum_hg >= SPECTRUM_G_PARAM_threshold_hg)
        {
            SPECTRUM_all_measurements[i].source_type = source_type_lg;
            SPECTRUM_all_measurements[i].sum = sum_lg;
            SPECTRUM_all_measurements[i].date = (SPECTRUM_G_acquisition_period_ptr+i)->date;
        }
        else
        {
            SPECTRUM_all_measurements[i].source_type = source_type_hg;
            SPECTRUM_all_measurements[i].sum = sum_hg;
            SPECTRUM_all_measurements[i].date = (SPECTRUM_G_acquisition_period_ptr+i)->date;
        }
    }

    return 1;
}


void SPECTRUM_energy_min_max(SPECTRUM__E_SOURCE_TYPE source_type)
{

    unsigned int min      = 0;
    unsigned int max      = 0;
    unsigned int i        = 0;
    unsigned int j        = 0;

    //get the first occurence of a given source_type in the energy sum array
    while(  SPECTRUM_all_measurements[j].source_type != source_type && \
            j < SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE)
    {
        j++;
    }
    
    max = SPECTRUM_all_measurements[j].sum;
    min = SPECTRUM_all_measurements[j].sum;

    //find min and max energy sum for a given source type
    for (i = j+1; i < SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE; i++)
    {
        if (SPECTRUM_all_measurements[i].source_type == source_type)
        {
            if (SPECTRUM_all_measurements[i].sum > max)
                max = SPECTRUM_all_measurements[i].sum;
            if (SPECTRUM_all_measurements[i].sum < min)
                min = SPECTRUM_all_measurements[i].sum;
        }
    }

    switch(source_type)
    {
        case SOURCE_TYPE_PHOTON:

            G_ENERGY_BOUNDS.ph_min_bound = min;
            G_ENERGY_BOUNDS.ph_max_bound = max;
        break;
        
        case SOURCE_TYPE_ELECTRON:

            G_ENERGY_BOUNDS.el_min_bound = min;
            G_ENERGY_BOUNDS.el_max_bound = max;
        break;

        case SOURCE_TYPE_UNDEFINED:

            G_ENERGY_BOUNDS.un_min_bound = min;
            G_ENERGY_BOUNDS.un_max_bound = max;
        break;

        default: //default case, implement safe exit
        break;
    }
}

unsigned int check_spectrum_index(unsigned int index_spectrum)
{
    if (index_spectrum > SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM)
        return SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM;
    else
        return index_spectrum; 
}

unsigned int* SP_generation_helper(void)
{
    static unsigned int delta_energy_arr[3];

    //Find min and max energies for each particle type
    SPECTRUM_energy_min_max(SOURCE_TYPE_PHOTON);
    SPECTRUM_energy_min_max(SOURCE_TYPE_ELECTRON);
    SPECTRUM_energy_min_max(SOURCE_TYPE_UNDEFINED);
    
    //calculate delta_energy = max - min, for the scaling of the spectrum histograms   
    delta_energy_arr[0] = G_ENERGY_BOUNDS.ph_max_bound - G_ENERGY_BOUNDS.ph_min_bound;
    delta_energy_arr[0] = delta_energy_arr[0] == 0 ? 1 : delta_energy_arr[0];

    delta_energy_arr[1] = G_ENERGY_BOUNDS.el_max_bound - G_ENERGY_BOUNDS.el_min_bound;
    delta_energy_arr[1] = delta_energy_arr[1] == 0 ? 1 : delta_energy_arr[1];

    delta_energy_arr[2] = G_ENERGY_BOUNDS.un_max_bound - G_ENERGY_BOUNDS.un_min_bound;
    delta_energy_arr[2] = delta_energy_arr[2] == 0 ? 1 : delta_energy_arr[2];     

    //calculate the number of bins for each particle type
    SPECTRUM_G_number_bin_ph = delta_energy_arr[0] / SPECTRUM_G_PARAM_spectrum_energy_step + 1;  
    if (SPECTRUM_G_number_bin_ph > SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM)
    {
        SPECTRUM_G_number_bin_ph = SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM;
    }

    SPECTRUM_G_number_bin_el = delta_energy_arr[1] / SPECTRUM_G_PARAM_spectrum_energy_step + 1;
    if (SPECTRUM_G_number_bin_el > SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM)
    {
        SPECTRUM_G_number_bin_el = SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM;
    }

    SPECTRUM_G_number_bin_un = delta_energy_arr[2] / SPECTRUM_G_PARAM_spectrum_energy_step + 1;
    if (SPECTRUM_G_number_bin_un > SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM)
    {
        SPECTRUM_G_number_bin_un = SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM;
    }

    return delta_energy_arr;
}

int SPECTRUM_spectrum_generation ( void )
{
    unsigned int index_spectrum    = 0;
    unsigned int energy_value      = 0;
    unsigned int            i      = 0;

    unsigned int *ptr_to_delta_en  = SP_generation_helper();

    unsigned int delta_energy_ph   = *(ptr_to_delta_en);
    unsigned int delta_energy_el   = *(ptr_to_delta_en+1);
    unsigned int delta_energy_un   = *(ptr_to_delta_en+2);

    /*
    Depending on the signal sum value, we increment the number
    of measured particle of the corresponding source type for
    the corresponding bin index
    */
    for (i = 0; i < SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE; i++)
    {   
        switch ((SPECTRUM_all_measurements[i]).source_type)
        {
            case SOURCE_TYPE_PHOTON:
            //subtracting from a given energy_sum[i] the minimum sum, normalizes the energy value 
                energy_value   = SPECTRUM_all_measurements[i].sum - G_ENERGY_BOUNDS.ph_min_bound;
                index_spectrum = ( (float) energy_value / delta_energy_ph) * SPECTRUM_G_number_bin_ph;
                index_spectrum = check_spectrum_index(index_spectrum);
                G_ONE_SPECTRUM_MEASUREMENT_OUTPUT.spectrum_particle_axis.photon_axis[index_spectrum] +=1;
            break;

            case SOURCE_TYPE_ELECTRON:
                energy_value   = SPECTRUM_all_measurements[i].sum - G_ENERGY_BOUNDS.el_min_bound;
                index_spectrum = ( (float) energy_value / delta_energy_el) * SPECTRUM_G_number_bin_el;
                index_spectrum = check_spectrum_index(index_spectrum);
                G_ONE_SPECTRUM_MEASUREMENT_OUTPUT.spectrum_particle_axis.electron_axis[index_spectrum] +=1; 
            break;

            case SOURCE_TYPE_UNDEFINED:
                energy_value   = SPECTRUM_all_measurements[i].sum - G_ENERGY_BOUNDS.un_min_bound;
                index_spectrum = ( (float) energy_value / delta_energy_un) * SPECTRUM_G_number_bin_un;
                index_spectrum = check_spectrum_index(index_spectrum);
                G_ONE_SPECTRUM_MEASUREMENT_OUTPUT.spectrum_particle_axis.undefined_axis[index_spectrum] +=1; 
            break;
            
            default:
            break;
        }
    }
    return 1;
}

void SPECTRUM_update_particle_axis(unsigned int i, SPECTRUM_T_particle_counts *particle_counts, unsigned short sum_flag)
{
    switch (SPECTRUM_all_measurements[i].source_type)
    {
        case SOURCE_TYPE_ELECTRON:
            if (sum_flag)
            {
                particle_counts->electron_counts += 1;
            }
            else
            {    
                particle_counts->electron_counts = 1;
            }
            break;
        case SOURCE_TYPE_PHOTON:
            if (sum_flag)
            {
                particle_counts->photon_counts += 1;
            }
            else
            {
                particle_counts->photon_counts = 1;
            }
            break;
        case SOURCE_TYPE_UNDEFINED:
            if (sum_flag)
            {
                particle_counts->undefined_counts += 1;
            }
            else
            {
                particle_counts->undefined_counts = 1;
            }
            break;
        default:
            break;
    }
}

void SPECTRUM_update_output_array(SPECTRUM_T_particle_counts *particle_counts)
{
    if (particle_counts->electron_counts > 0)
        G_ONE_LIGHT_MEASUREMENT_OUTPUT.light_particle_axis.electron_axis [SPECTRUM_G_CNST_lc_array_size] = \
        particle_counts->electron_counts;

    if (particle_counts->photon_counts > 0)
        G_ONE_LIGHT_MEASUREMENT_OUTPUT.light_particle_axis.photon_axis   [SPECTRUM_G_CNST_lc_array_size] = \
        particle_counts->photon_counts;

    if (particle_counts->undefined_counts > 0)
        G_ONE_LIGHT_MEASUREMENT_OUTPUT.light_particle_axis.undefined_axis[SPECTRUM_G_CNST_lc_array_size] = \
        particle_counts->undefined_counts;
}

void SPECTRUM_light_curve_generation( void )
{
    unsigned int index1                 = 0;
    unsigned int index2                 = 0;
    unsigned int acq_duration           = 0;

    float time_step                     = 0; //Change to the same type as acq_duration
    
    acq_duration = SP_G_CNST_acq_duration;
    
    if (acq_duration >  SPECTRUM_all_measurements[SPECTRUM__C_MAX_BIN_NUMBER_LIGHT_CURVE-1].date - \
                        SPECTRUM_all_measurements[0].date)
        {
            acq_duration =  SPECTRUM_all_measurements[SPECTRUM__C_MAX_BIN_NUMBER_LIGHT_CURVE-1].date -\
                            SPECTRUM_all_measurements[0].date;  
        }
    
    SPECTRUM_G_CNST_lc_step = acq_duration / SPECTRUM_G_PARAM_number_bin_lc;
    if (SPECTRUM_G_CNST_lc_step == 0)
        SPECTRUM_G_CNST_lc_step = 1;
    
    G_ONE_LIGHT_MEASUREMENT_OUTPUT.SPECTRUM_G_CNST_lc_step = SPECTRUM_G_CNST_lc_step; 
    SPECTRUM_T_particle_counts particle_counts = {0};
    
    index2 = 0;
    for (index1 = 0; index1 < SPECTRUM__C_MAX_BIN_NUMBER_LIGHT_CURVE; index1++)
    {
        //calculate the time step
        if(index1 < SPECTRUM__C_MAX_BIN_NUMBER_LIGHT_CURVE-1)
        {
            time_step = SPECTRUM_all_measurements[index1+1].date - SPECTRUM_all_measurements[index2].date;
        }
        else
        {
            time_step = SPECTRUM_all_measurements[index1].date - SPECTRUM_all_measurements[index2].date;
            G_ONE_LIGHT_MEASUREMENT_OUTPUT.last_time_step = time_step;
        }
        //count the number of particles deteceted during a given time_step   
        if(time_step < SPECTRUM_G_CNST_lc_step)
        {
            if (index1 == 0 || index1 == SPECTRUM__C_MAX_BIN_NUMBER_LIGHT_CURVE)
            {
                SPECTRUM_update_particle_axis(index1, &particle_counts,1);
            }
            else
            { 
                SPECTRUM_update_particle_axis(index1+1, &particle_counts,1);
            }
        }
        else
        {
            if (index1 == 0)
            {
                SPECTRUM_update_particle_axis(index1, &particle_counts, 0);
            }
            //if a new time_step > LC_time_step, update LC output array
            //with the number of counted particles
            SPECTRUM_update_output_array(&particle_counts);
            SPECTRUM_G_CNST_lc_array_size++;
            index2 = index1+1;
            
            //resetr the particle counts
            particle_counts.electron_counts    = 0;
            particle_counts.photon_counts      = 0;
            particle_counts.undefined_counts   = 0;
            
            //if a new time_step > LC_time_step, we detect a new particle
            if (index1 <= SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE-1)
            {
                SPECTRUM_update_particle_axis(index1+1, &particle_counts, 0);
            }
            else
            {
                SPECTRUM_update_particle_axis(index1, &particle_counts, 0);
            }
        }
    }
    SPECTRUM_update_output_array(&particle_counts);
    SPECTRUM_G_CNST_lc_array_size++;
    //check if the lc_array_size is within the array size
    if (SPECTRUM_G_CNST_lc_array_size >= SPECTRUM__C_MAX_BIN_NUMBER_LIGHT_CURVE)
        SPECTRUM_G_CNST_lc_array_size = SPECTRUM__C_MAX_BIN_NUMBER_LIGHT_CURVE;    
}

void SPECTRUM_update_output_array2( SPECTRUM__E_SOURCE_TYPE source_type,
                                    SPECTRUM_T_LC_PER_SP_TIME *part_type,
                                    SPECTRUM_T_particle_counts *particle_counts,
                                    unsigned int lc_array_size)
{   
    switch (source_type)
    {
        case SOURCE_TYPE_ELECTRON:
            if (particle_counts->electron_counts > 0)
            {
                part_type->num_part_per_dt[lc_array_size] = particle_counts->electron_counts;  
            }
        break;

        case SOURCE_TYPE_PHOTON:
            if (particle_counts->photon_counts > 0)
            {
                part_type->num_part_per_dt[lc_array_size] = particle_counts->photon_counts;  
            }
        break;

        case SOURCE_TYPE_UNDEFINED:
            if (particle_counts->undefined_counts > 0)
            {
                part_type->num_part_per_dt[lc_array_size] = particle_counts->undefined_counts;  
            }
        break;
    }
}

void SPECTRUM_update_particle_axis2(SPECTRUM__E_SOURCE_TYPE source_type,
                                    SPECTRUM_T_particle_counts *particle_counts,
                                    unsigned short sum_flag)
{
    switch (source_type)
    {
        case SOURCE_TYPE_PHOTON:
            if (sum_flag)
            {
                particle_counts->photon_counts += 1;
            }
            else
            {    
                particle_counts->photon_counts = 1;
            }
        break;

        case SOURCE_TYPE_ELECTRON:
            if (sum_flag)
            {
                particle_counts->electron_counts += 1;
            }
            else
            {
                particle_counts->electron_counts = 1;
            }
        break;

        case SOURCE_TYPE_UNDEFINED:
            if (sum_flag)
            {
                particle_counts->undefined_counts += 1;
            }
            else
            {    
                particle_counts->undefined_counts = 1;
            }
        break;
    }
}

unsigned int* check_if_index_spectrum_is_in_energy_array(unsigned int index_to_search)
{
    unsigned int i;
    static unsigned int array[2];

    i        = 0;
    array[0] = 0;
    array[1] = 0;
    for(i = 0; i < SPECTRUM_PARAM_LC_per_SP_arr_size; i++)
    {
        if(PARAM_LC_per_SP_ENERGY_values[i] == index_to_search)
        {
            array[0] = 1;
            array[1] = i;
            break;
        }
    }
    return array;
}

void SP_LC_per_energy_time_regrouping(SPECTRUM_T_LC_PER_SP_TIME *part_type, SPECTRUM__E_SOURCE_TYPE source_type)
{

    unsigned int index1           = 0;
    unsigned int index2           = 0;
    unsigned int index_shift      = 0;
    unsigned int acq_duration     = 0;
    unsigned int lc_array_size    = 0;

    float time_step               = 0; //to change time_step from float to unsigned int

    acq_duration = SP_G_CNST_acq_duration;
    index_shift  = part_type->index_shift == 0 ? 1 : part_type->index_shift;

    //verify for the validity of time during which the particles were detected
    if( acq_duration > part_type->time_array[index_shift-1] - part_type->time_array[0] )
    {
        acq_duration = part_type->time_array[index_shift-1] - part_type->time_array[0];
    }

    //calculate the light curve step for each particle and each energy
    SPECTRUM_G_CNST_lc_step =  acq_duration / SPECTRUM_G_PARAM_number_bin_lc;        
   
    if ( SPECTRUM_G_CNST_lc_step == 0 )
    {
        SPECTRUM_G_CNST_lc_step = 1;
    }
    //get the time step of a given particle
    part_type->time_step = SPECTRUM_G_CNST_lc_step;
    SPECTRUM_T_particle_counts particle_counts = {0};
    index2 = 0;
    for (index1 = 0; index1 < index_shift; index1++)
    {
        //calculate the time step
        if(index1 < index_shift-1)
        {
            time_step = part_type->time_array[index1+1] - part_type->time_array[index2];
        }
        else
        {
            time_step = part_type->time_array[index1] - part_type->time_array[index2];
        }

        //count the number of particles detected during a given time_step
        if(time_step < SPECTRUM_G_CNST_lc_step)
        {
            SPECTRUM_update_particle_axis2(source_type, &particle_counts, 1);
        }
        //if a total time step for a given particle is greater than lc_step   
        else
        {
            if (index1 == 0)
            {
                SPECTRUM_update_particle_axis2(source_type, &particle_counts,0);
            }
            //if a news time_step > LC_time_step, update LC output array with
            //the number of counted particles 
            SPECTRUM_update_output_array2(source_type, part_type, &particle_counts, lc_array_size);
            lc_array_size++;
            index2 = index1+1;
            
            //reset all particle counts
            particle_counts.electron_counts = 0;
            particle_counts.photon_counts = 0;
            particle_counts.undefined_counts = 0;
            
            //we still detect a new particle
            SPECTRUM_update_particle_axis2(source_type, &particle_counts, 0);
        }
    }

    SPECTRUM_update_output_array2(source_type, part_type, &particle_counts, lc_array_size);
    lc_array_size++;
    //verify if lc array size is within max array size
    if (lc_array_size >= SPECTRUM__C_MAX_BIN_NUMBER_LIGHT_CURVE)
    {
        lc_array_size = SPECTRUM__C_MAX_BIN_NUMBER_LIGHT_CURVE;
    }
    //update the max array size
    part_type->max_arr_size = lc_array_size;
}

void SP_classify_energies(unsigned int i, unsigned int index_sp, SPECTRUM__E_SOURCE_TYPE src_type)
{

    unsigned int j              = 0;
    unsigned int k              = 0;
    unsigned int sparse_idx     = 0;
    unsigned int index_shift    = 0;
    unsigned int index_spectrum = 0;
    unsigned int num_cons_zeros = 0;
    unsigned int *ptr_to_arr;

    //check if the index spectrum is valid, within energy bounds
    index_spectrum = check_spectrum_index(index_sp);
    //check if a given energy index, of a given particle and energy value
    //is in the energy PARAM_LC_per_SP_ENERGY_values array
    ptr_to_arr = check_if_index_spectrum_is_in_energy_array(index_spectrum);    
    
    //if a given energy is in the energy array
    if ((*ptr_to_arr) == 1)
    {
        //check if array length is less than SP__C_MAX_LC_PER_EN_ARR_LEN
        index_shift = G_LC_PER_SP_energy[*(ptr_to_arr+1)].ph_el_un_arr[src_type].index_shift;
        sparse_idx  = G_LC_PER_SP_energy[*(ptr_to_arr+1)].ph_el_un_arr[src_type].sparse_array_len;
        
        if (index_shift >= SP__C_MAX_LC_PER_EN_ARR_LEN || sparse_idx >= SP__C_MAX_LC_PER_EN_ARR_LEN)
        {
            printf("SP__C_MAX_LC_PER_EN_ARR_LEN is SATURATED increase it's size\n");
            G_LC_PER_SP_energy[*(ptr_to_arr+1)].ph_el_un_arr[src_type].index_shift = SP__C_MAX_LC_PER_EN_ARR_LEN-1;
            G_LC_PER_SP_energy[*(ptr_to_arr+1)].ph_el_un_arr[src_type].sparse_array_len  = SP__C_MAX_LC_PER_EN_ARR_LEN-1;
        }
        
        //update the time array, by the detection time for the detected particle and its energy
        G_LC_PER_SP_energy[*(ptr_to_arr+1)].ph_el_un_arr[src_type].time_array[index_shift] = SPECTRUM_all_measurements[i].date;
        G_LC_PER_SP_energy[*(ptr_to_arr+1)].ph_el_un_arr[src_type].index_shift += 1;

        //for each particle type and each energy get number of consecutives zeros (i.e. the number of time steps 
        //that the given particle of a given energy wasn't detected)
        num_cons_zeros = G_LC_PER_SP_energy[*(ptr_to_arr+1)].ph_el_un_arr[src_type].struct_num_consec_zeros; 
        G_LC_PER_SP_energy[*(ptr_to_arr+1)].ph_el_un_arr[src_type].sparse_array[sparse_idx] = num_cons_zeros;
        G_LC_PER_SP_energy[*(ptr_to_arr+1)].ph_el_un_arr[src_type].sparse_array_len += 1;
        G_LC_PER_SP_energy[*(ptr_to_arr+1)].ph_el_un_arr[src_type].struct_num_consec_zeros = 0;

        //for all the other particle types and energies, that weren't detected, update the nubmer  of
        //consecutive zeros (i.e. the number of time steps that we haven't saw a given energy) 
        for (j = 0; j < SPECTRUM_PARAM_LC_per_SP_arr_size; j++)
        {
            for (k = 0; k < 3; k++)
            {
                   if (j != *(ptr_to_arr+1) || k != src_type)
                    G_LC_PER_SP_energy[j].ph_el_un_arr[k].struct_num_consec_zeros += 1;
                else
                    continue;
            }
        }
    }
    //if particle isn't in energy array, for all particles update the nubmer  of
    //consecutive zeros
    else
    {
        for (j = 0; j < SPECTRUM_PARAM_LC_per_SP_arr_size; j++)
        {
            for (k = 0; k < 3; k++)
            {
                G_LC_PER_SP_energy[j].ph_el_un_arr[k].struct_num_consec_zeros += 1;
            }
        }
    }
}

void SP_LC_per_energy_generation( void )
{
    unsigned int i              = 0;
    unsigned int j              = 0;
    unsigned int index_spectrum = 0;
    unsigned int energy_value   = 0;

    unsigned int *ptr_to_delta_en = SP_generation_helper();

    unsigned int delta_energy_ph = *(ptr_to_delta_en);
    unsigned int delta_energy_el = *(ptr_to_delta_en+1);
    unsigned int delta_energy_un = *(ptr_to_delta_en+2);

    //initialize the SPECTRUM_T_LC_PER_SP_ENERGY STRUCT
    for (i = 0; i < SPECTRUM_PARAM_LC_per_SP_arr_size; i++)
    {
        for (j = 0; j < 3; j++)
        {
            G_LC_PER_SP_energy[i].ph_el_un_arr[j].index_shift = 0;
            G_LC_PER_SP_energy[i].ph_el_un_arr[j].max_arr_size = 0;
            G_LC_PER_SP_energy[i].ph_el_un_arr[j].sparse_array_len = 0;
            G_LC_PER_SP_energy[i].ph_el_un_arr[j].struct_num_consec_zeros = 0;
        }  
    }

    //according to the particle type, calculate the ith detected particle energy bin 
    for (i = 0; i < SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE; i++)    
    {   
        switch(SPECTRUM_all_measurements[i].source_type)
        {
            case SOURCE_TYPE_PHOTON:
                energy_value   = SPECTRUM_all_measurements[i].sum - G_ENERGY_BOUNDS.ph_min_bound;
                index_spectrum = ((float) energy_value / delta_energy_ph) * SPECTRUM_G_number_bin_ph;
                SP_classify_energies(i, index_spectrum, SOURCE_TYPE_PHOTON);
            break;

            case SOURCE_TYPE_ELECTRON:
                energy_value   = SPECTRUM_all_measurements[i].sum - G_ENERGY_BOUNDS.el_min_bound;
                index_spectrum = ((float) energy_value / delta_energy_el) * SPECTRUM_G_number_bin_el;
                SP_classify_energies(i, index_spectrum, SOURCE_TYPE_ELECTRON);
            break;
        
            case SOURCE_TYPE_UNDEFINED:
                energy_value   = SPECTRUM_all_measurements[i].sum - G_ENERGY_BOUNDS.un_min_bound;
                index_spectrum = ((float) energy_value / delta_energy_un) * SPECTRUM_G_number_bin_un;
                SP_classify_energies(i, index_spectrum, SOURCE_TYPE_UNDEFINED);
            break;

            default:
            break;
        }
    }

    // for all energies in defined energy array regroup their time accorcind to the time step
    for (i = 0; i < SPECTRUM_PARAM_LC_per_SP_arr_size; i++)
    {
        SP_LC_per_energy_time_regrouping( &(G_LC_PER_SP_energy[i].ph_el_un_arr[SOURCE_TYPE_PHOTON]),   SOURCE_TYPE_PHOTON );
        SP_LC_per_energy_time_regrouping( &(G_LC_PER_SP_energy[i].ph_el_un_arr[SOURCE_TYPE_ELECTRON]), SOURCE_TYPE_ELECTRON);
        SP_LC_per_energy_time_regrouping( &(G_LC_PER_SP_energy[i].ph_el_un_arr[SOURCE_TYPE_UNDEFINED]),SOURCE_TYPE_UNDEFINED);
    }
}

int SPECTRUM_process( void )
{

    #ifdef USE_INPUT_FILE
        INPUTOUTPUT_read_test_data_from_file(0,SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE);
    #else
        INPUTOUTPUT_read_test_data_from_array(0,SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE);
    #endif

    //verify acquired data, this function should be called
    //before all the other function below
    SPECTRUM_data_treatement();
    
    //UNCOMMENT THE FUNCTIONS BELOW THE LINE TO TEST LC PER ENERGY ALGORITHM
    //---------------------------//   
    // SP_LC_per_energy_generation();
    // print_lc_per_energy_detection_summary();
    // INPUTOUTPUT_write_lc_per_energy_to_file();
    //---------------------------//

    //UNCOMMENT THE FUNCTION BELOW THE LINE TO TEST SPECTRUM ALGORITHM
    //---------------------------//
    SPECTRUM_spectrum_generation();
    //---------------------------//

    //UNCOMMENT THE FUNCTION BELOW THE LINE TO TEST LC (FOR TOTAL ENERGIES) ALGORITHM
    //---------------------------//
    SPECTRUM_light_curve_generation();
    //---------------------------//
    
    print_particle_detection_summary();
    // INPUTOUTPUT_write_LC_to_file();
    // INPUTOUTPUT_write_SP_to_file();
    
    return (0);
}

void print_particle_detection_summary(void)
{
    unsigned int i               = 0;
    unsigned int count_particles = 0;

    printf("------- TOTAL NUMBER OF DETECTED PARTICLES -------\n");
    printf("PHOTON = 0\t ELECTRON = 1\t UNDEFINED = 2\t\n");
    printf("--------------------------------------------------\n");
    for(i = 0; i < SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE; i++)
    {
         printf("type = %d\t date = %f\t\n",SPECTRUM_all_measurements[i].source_type,SPECTRUM_all_measurements[i].date);
    }
    
    printf("---------------LIGHT CURVES ------------------\n");
    printf("------------------Δt = %d --------------------\n",SPECTRUM_G_CNST_lc_step);
    printf("----------------------------------------------\n");
    for (i = 0; i < SPECTRUM_G_CNST_lc_array_size; i++)
    {
        printf("photon = %d\t electron = %d\t undefined = %d\t\n",
                G_ONE_LIGHT_MEASUREMENT_OUTPUT.light_particle_axis.photon_axis[i],
                G_ONE_LIGHT_MEASUREMENT_OUTPUT.light_particle_axis.electron_axis[i],
                G_ONE_LIGHT_MEASUREMENT_OUTPUT.light_particle_axis.undefined_axis[i]);

        count_particles += G_ONE_LIGHT_MEASUREMENT_OUTPUT.light_particle_axis.electron_axis[i];
        count_particles += G_ONE_LIGHT_MEASUREMENT_OUTPUT.light_particle_axis.photon_axis[i];
        count_particles += G_ONE_LIGHT_MEASUREMENT_OUTPUT.light_particle_axis.undefined_axis[i];
    }
    if(count_particles != SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE)
    {
        printf("light curve particle = %d\n", count_particles);
        printf("total particles = %d\n", SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE);
        printf("ERROR in SPECTRUM_light_curve_generation\n");
    }

    printf("----------------------------------------------\n");
    printf("---------------- SPECTRUM HISTOGRAMS ---------\n");
    printf("----------------------------------------------\n");
    unsigned int iterator = SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM;
    if (SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE < SPECTRUM__C_MAX_BIN_NUMBER_SPECTRUM)
        iterator = SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE;
    for (i = 0; i < iterator; i++)
    {
        printf("ENERGY LEVEL = %d\t photon = %d\t electron = %d\t undefined = %d\t\n",
                i,
                G_ONE_SPECTRUM_MEASUREMENT_OUTPUT.spectrum_particle_axis.photon_axis[i], 
                G_ONE_SPECTRUM_MEASUREMENT_OUTPUT.spectrum_particle_axis.electron_axis[i],
                G_ONE_SPECTRUM_MEASUREMENT_OUTPUT.spectrum_particle_axis.undefined_axis[i]);
    }
}

void print_lc_per_energy_detection_summary( void )
{
    unsigned int i;
    unsigned int j;
    // int count_particles = 0;
    printf("\n");
    printf("----------LIGHT CURVES PER ENERGY ------------\n");
    printf("------------------Δt = %d --------------------\n",SPECTRUM_G_CNST_lc_step);
    printf("---------------------------------------------\n");
    printf("\n");

    for (int src_type = 0; src_type < 3; src_type++)
    {
        printf("---------------------------------------------\n");
                if (src_type == 0)
                    printf("-------------------PHOTONS--------------------\n");
                else if(src_type == 1)
                    printf("----------------ELECTRONS--------------------\n");
                else
                    printf("----------------UNDEFINED--------------------\n");
        printf("---------------------------------------------\n");

        for (i = 0; i < SPECTRUM_PARAM_LC_per_SP_arr_size; i++)
        {
            // count_particles = 0;
            printf("---------------------------------------------\n");
            printf("--------------ENERGY VALUE = %d--------------\n", PARAM_LC_per_SP_ENERGY_values[i]);
            printf("---------------------------------------------\n");
            // for (j = 0; j < G_LC_PER_SP_energy[i].ph_el_un_arr[1].max_arr_size; j++)
            // {
            //     count_particles += G_LC_PER_SP_energy[i].ph_el_un_arr[1].num_part_per_dt[j];
            //     printf("electron %d\t\n", G_LC_PER_SP_energy[i].ph_el_un_arr[1].num_part_per_dt[j]);
            // }
            // printf("----------total_particle_count = %d----------\n", count_particles);
            // printf("---------------------------------------------\n");
            // printf("\n");
            printf("sparse_array_len = %d\n",G_LC_PER_SP_energy[i].ph_el_un_arr[src_type].sparse_array_len);
            printf("index_shift = %d\n",G_LC_PER_SP_energy[i].ph_el_un_arr[src_type].index_shift);
            printf("max_arr_size = %d\n",G_LC_PER_SP_energy[i].ph_el_un_arr[src_type].max_arr_size);
            
            for (j = 0; j < G_LC_PER_SP_energy[i].ph_el_un_arr[src_type].max_arr_size; j++)
            {   
                printf("num_part_per_dt %d\t num_consec_zeros %d\n",
                G_LC_PER_SP_energy[i].ph_el_un_arr[src_type].num_part_per_dt[j],
                G_LC_PER_SP_energy[i].ph_el_un_arr[src_type].sparse_array[j]);
            }
    
        }
    }
}
unsigned int* SPECTRUM_get_threshold_hg( void )
{
    return &SPECTRUM_G_PARAM_threshold_hg;
}

unsigned int* SPECTRUM_get_high_threshold( void )
{
    return &SPECTRUM_G_PARAM_high_thresh_HG;
}

unsigned int* SPECTRUM_get_low_threshold( void )
{
    return &SPECTRUM_G_PARAM_low_thresh_HG;
}

unsigned int* SPECTRUM_get_energy_step( void )
{
    return &SPECTRUM_G_PARAM_spectrum_energy_step;
}

unsigned int*  SPECTRUM_get_lc_array_size(void)
{
    return &SPECTRUM_G_CNST_lc_array_size;
}

unsigned int* SPECTRUM_get_number_of_bin_ph( void )
{
    return &SPECTRUM_G_number_bin_ph;
}

unsigned int* SPECTRUM_get_number_of_bin_el( void )
{
    return &SPECTRUM_G_number_bin_el;
}

unsigned int* SPECTRUM_get_number_of_bin_un( void )
{
    return &SPECTRUM_G_number_bin_un;
}

unsigned int* SPECTRUM_get_G_CNST_lc_step( void )
{
    return &SPECTRUM_G_CNST_lc_step;
}

SPECTRUM__T_SPECTRUM_ENERGY_BOUNDS* SPECTRUM_G_get_energy_bounds(void)
{
    return &G_ENERGY_BOUNDS;
}

SPECTRUM__T_SPECTRUM_MEASUREMENT_OUTPUT* SPECTRUM_G_get_spectrum_measurement_output( void )
{
    return &G_ONE_SPECTRUM_MEASUREMENT_OUTPUT;
}

SPECTRUM__T_LIGHT_MEASUREMENT_OUTPUT* SPECTRUM_G_get_light_measurement_output( void )
{
    return &G_ONE_LIGHT_MEASUREMENT_OUTPUT;
}

unsigned int*  SPECTRUM_get_offset_array( void )
{
    return &SPECTRUM_G_PARAM_offset[0];
}

unsigned int* SPECTRUM_get_gain_array( void )
{
    return &SPECTRUM_G_PARAM_gain[0];
}

SPECTRUM__T_ALL_CHANNEL_ADC_VALUE* SPECTRUM_G_get_acquisition_period( void )
{
    return &SPECTRUM_G_acquisition_period[0];
}

SPECTRUM__T_ALL_CHANNEL_ADC_VALUE* SPECTRUM_get_acquisition_period_array(void)
{
    return &SPECTRUM_G_acquisition_period[0];
}

SPECTRUM_T_LC_PER_SP_ENERGY* SP_get_lc_per_energy_array(void)
{
    return &G_LC_PER_SP_energy[0];
}
unsigned short* SP_get_energies_per_LC( void )
{
    return &PARAM_LC_per_SP_ENERGY_values[0];
}