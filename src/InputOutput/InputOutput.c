#include "../spectrum/spectrum.h"
#include "../spectrum/array_test_data/sample.h"

//------------------------------------------------------------------------------
//         Function definition
//------------------------------------------------------------------------------

double random_uniform_generator( void )
{
    return (double)rand()/(double)(RAND_MAX); 
}

double random_exponential_generator(double lambda)
{
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1 - u) / lambda;
}

void INPUTOUTPUT_read_test_data_from_file(unsigned int start_index,unsigned int end_index)
{
    unsigned int column_counter  = 0;
    unsigned int line_counter    = 0;
    unsigned int line_number     = 0;

    unsigned short adc_value_hg;
    unsigned short adc_value_lg;

    FILE *fptr_hg = NULL;
    FILE *fptr_lg = NULL;
    
    fptr_hg = fopen("src/spectrum/file_test_data/ADC_ReadOut_HG_Co60.txt", "r");
    fptr_lg = fopen("src/spectrum/file_test_data/ADC_ReadOut_LG_Co60.txt", "r");
    
    if(fptr_hg == NULL || fptr_lg == NULL)
    {   
        exit(EXIT_FAILURE);
    }

    SPECTRUM__T_ALL_CHANNEL_ADC_VALUE* acquisition_period = SPECTRUM_G_get_acquisition_period();

    //Read adc hg and lg values unitl the end of the acquisition line
    //Example : NUMBER_LIGHT_CURVE = 2 ; ENERGY_MEASUREMENT_ARRAY_SIZE = 10
    //(Acquisition 1) Will read and acquire the first 10 lines
    //(Acquisition 2) Will read but not acquire the first 10 lines, then
    //                will read and acquire the lines in a range [10, 11, ... ,20]
    
    while(fscanf(fptr_hg, "%hd", &adc_value_hg) && fscanf(fptr_lg, "%hd", &adc_value_lg) && line_counter < end_index)
    {
        column_counter = column_counter % SCI_COMMON__C_NUMBER_OF_SIPM;
        if (line_counter >= start_index) 
        {
            line_number = line_counter % SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE;
            acquisition_period[line_number].one_period_value[column_counter].id = column_counter;
            acquisition_period[line_number].one_period_value[column_counter].HG_value = adc_value_hg; 
            acquisition_period[line_number].one_period_value[column_counter].LG_value = adc_value_lg;

            if (column_counter+1 == SCI_COMMON__C_NUMBER_OF_SIPM)
            {
                acquisition_period[line_number].date = start_index + line_number;
            }
        }
        if (column_counter+1 == SCI_COMMON__C_NUMBER_OF_SIPM)
        {
            line_counter++;
        }
        column_counter++;
    }
    
    fclose(fptr_hg);
    fclose(fptr_lg);
}

//Description: Reads data from an array, ranging in [start_index, end_index]
void INPUTOUTPUT_read_test_data_from_array(unsigned int start_index, unsigned int end_index)
{

    unsigned int column_counter1 = 0;
    //allows to idnex at most start_index*26 entries of an array
    unsigned int column_counter2 = start_index*SCI_COMMON__C_NUMBER_OF_SIPM;
    unsigned int line_counter    = 0;
    unsigned int line_number     = 0;

    unsigned short * adc_value_hg_arr = SAMPLE_DATA_getter_HG();
    unsigned short * adc_value_lg_arr = SAMPLE_DATA_getter_LG();

    //Get SPECTRUM__T_ALL_CHANNEL_ADC_VALUE array struct that containts all acquisitions
    SPECTRUM__T_ALL_CHANNEL_ADC_VALUE* acquisition_period = SPECTRUM_G_get_acquisition_period();
    
    //Acquires the data from a 1D array 
    for (line_number = start_index; line_number < end_index; line_number++)
    {
            column_counter1 = 0;
            line_counter = line_number % SPECTRUM__C_ALL_ENERGY_MEASUREMENT_ARRAY_SIZE;    

            while(column_counter1 < SCI_COMMON__C_NUMBER_OF_SIPM)
            {
                acquisition_period[line_counter].one_period_value[column_counter1].id = column_counter1;
                acquisition_period[line_counter].one_period_value[column_counter1].HG_value = adc_value_hg_arr[column_counter2]; 
                acquisition_period[line_counter].one_period_value[column_counter1].LG_value = adc_value_lg_arr[column_counter2];

                column_counter1++;
                column_counter2++;
            }

        // if (line_counter < 1000)
        // {
        //     if(line_counter == 0)
        //         acquisition_period[line_counter].date = random_exponential_generator(20);
        //     else
        //         acquisition_period[line_counter].date = random_exponential_generator(20) + acquisition_period[line_counter-1].date;
        // }
        // else if (line_counter > 1000 && line_counter < 4000)
        // {
        //     acquisition_period[line_counter].date = random_exponential_generator(40) + acquisition_period[line_counter-1].date;
        // }
        // else if (line_counter > 4000 && line_counter < 9500)
        // {
        //     acquisition_period[line_counter].date = random_exponential_generator(20) + acquisition_period[line_counter-1].date;   
        // }
        // else
        // {
        //     acquisition_period[line_counter].date = random_exponential_generator(10) + acquisition_period[line_counter-1].date;      
        // }
        
        if(line_counter == 0)
            acquisition_period[line_counter].date = random_exponential_generator(20);
        else
            acquisition_period[line_counter].date = random_exponential_generator(20) + acquisition_period[line_counter-1].date;
        
    }
    printf("Total acquisition time = %f\n", acquisition_period[line_counter].date);
}

void INPUTOUTPUT_write_LC_to_file( void )
{
    unsigned int SPECTRUM_G_lc_array_size = *SPECTRUM_get_lc_array_size();
    unsigned int index1 = 0;
    SPECTRUM__T_LIGHT_MEASUREMENT_OUTPUT* one_light_measurement =  SPECTRUM_G_get_light_measurement_output();

    FILE *file_pointer_electron;
    FILE *file_pointer_photon;
    FILE *file_pointer_undefined;
    FILE *file_time_stamp;   


    file_pointer_electron   = fopen("output/LC/electron.txt", "w");
    file_pointer_photon     = fopen("output/LC/photon.txt", "w");
    file_pointer_undefined  = fopen("output/LC/undefined.txt", "w");
    file_time_stamp         = fopen("output/LC/time_stamp.txt", "w");

    if(file_pointer_photon == NULL || file_pointer_electron == NULL || file_pointer_undefined == NULL || file_time_stamp == NULL) 
    {
        printf("Writing data error!");
        exit(1);
    }

    for (index1 = 0; index1 < SPECTRUM_G_lc_array_size; index1++)   
    {
        fprintf(file_pointer_electron,"%d\n", one_light_measurement->light_particle_axis.electron_axis[index1]);
        fprintf(file_pointer_photon,"%d\n", one_light_measurement->light_particle_axis.photon_axis[index1]);
        fprintf(file_pointer_undefined,"%d\n", one_light_measurement->light_particle_axis.undefined_axis[index1]);        
    }
    
    fprintf(file_time_stamp,"%d\n",one_light_measurement->SPECTRUM_G_CNST_lc_step);
    fprintf(file_time_stamp,"%f\n",one_light_measurement->last_time_step);

    fclose(file_pointer_electron);
    fclose(file_pointer_photon);
    fclose(file_pointer_undefined);
    fclose(file_time_stamp);
}

void INPUTOUTPUT_write_SP_to_file(void)
{
    SPECTRUM__T_SPECTRUM_MEASUREMENT_OUTPUT* one_spectrum_measurement = SPECTRUM_G_get_spectrum_measurement_output();
    unsigned int number_of_bins_ph = *SPECTRUM_get_number_of_bin_ph();
    unsigned int number_of_bins_el = *SPECTRUM_get_number_of_bin_el();
    unsigned int number_of_bins_un = *SPECTRUM_get_number_of_bin_un();
    unsigned int index1 = 0;
    FILE *file_pointer_electron;
    FILE *file_pointer_photon;
    FILE *file_pointer_undefined;

    file_pointer_electron   = fopen("output/SP/electron.txt", "w");
    file_pointer_photon     = fopen("output/SP/photon.txt", "w");
    file_pointer_undefined  = fopen("output/SP/undefined.txt", "w");

    if(file_pointer_photon == NULL || file_pointer_electron == NULL || file_pointer_undefined == NULL) 
    {
        printf("Writing data error!");
        exit(1);
    }

    for (index1 = 0; index1 < number_of_bins_ph; index1++)   
    {
        fprintf(file_pointer_photon,"%d\n",    one_spectrum_measurement->spectrum_particle_axis.photon_axis[index1]);
    }
     for (index1 = 0; index1 < number_of_bins_el; index1++)   
    {
        fprintf(file_pointer_electron,"%d\n",  one_spectrum_measurement->spectrum_particle_axis.electron_axis[index1]);
    }   

    for (index1 = 0; index1 < number_of_bins_un; index1++)   
    {
        fprintf(file_pointer_undefined,"%d \n", one_spectrum_measurement->spectrum_particle_axis.undefined_axis[index1]);
    }

    fclose(file_pointer_electron);
    fclose(file_pointer_photon);
    fclose(file_pointer_undefined);

}

void INPUTOUTPUT_write_lc_per_energy_to_file( void )
{
    SPECTRUM_T_LC_PER_SP_ENERGY* ptr_to_lc_per_energy = SP_get_lc_per_energy_array();
    unsigned short*              ptr_to_all_energies  = SP_get_energies_per_LC();
    unsigned int src_type   = 0; 
    unsigned int index1     = 0;
    unsigned int index2     = 0;

    FILE *file_ptr_el;
    FILE *file_ptr_ph;
    FILE *file_ptr_un;
    FILE *file_time_stamp;

    file_ptr_el     = fopen("output/LC_PER_ENERGY/electron.txt", "w");
    file_ptr_ph     = fopen("output/LC_PER_ENERGY/photon.txt", "w");
    file_ptr_un     = fopen("output/LC_PER_ENERGY/undefined.txt", "w");
    file_time_stamp = fopen("output/LC_PER_ENERGY/time_stamp.txt", "w");

    if(file_ptr_ph == NULL || file_ptr_el == NULL || file_ptr_un == NULL || file_time_stamp == NULL) 
    {
        printf("Writing data error!");
        exit(1);
    }

    for (src_type = 0; src_type < 3; src_type++)   
    {
        for (index1 = 0; index1 < SPECTRUM_PARAM_LC_per_SP_arr_size; index1++)
        {
            fprintf(file_time_stamp,"%f %d %d\n",
                                    ptr_to_lc_per_energy[index1].ph_el_un_arr[src_type].time_step,
                                    ptr_to_all_energies[index1],
                                    src_type);
            //write time data
            for (index2 = 0; index2 < ptr_to_lc_per_energy[index1].ph_el_un_arr[src_type].max_arr_size; index2++)
            {
                switch(src_type)
                {
                    case SOURCE_TYPE_PHOTON:
                    fprintf(file_ptr_ph,"%d\t %d\n",
                        ptr_to_lc_per_energy[index1].ph_el_un_arr[src_type].num_part_per_dt[index2],
                        ptr_to_lc_per_energy[index1].ph_el_un_arr[src_type].sparse_array[index2]);            
                    break;
                    case SOURCE_TYPE_ELECTRON:
                    fprintf(file_ptr_el,"%d\t %d\n",
                        ptr_to_lc_per_energy[index1].ph_el_un_arr[src_type].num_part_per_dt[index2],
                        ptr_to_lc_per_energy[index1].ph_el_un_arr[src_type].sparse_array[index2]);
                    break;
                    case SOURCE_TYPE_UNDEFINED:
                    fprintf(file_ptr_un,"%d\t %d\n", 
                        ptr_to_lc_per_energy[index1].ph_el_un_arr[src_type].num_part_per_dt[index2],
                        ptr_to_lc_per_energy[index1].ph_el_un_arr[src_type].sparse_array[index2]);
                    break;
                }
            }

            //separate each energy by a white space
            switch (src_type)
            {
                case SOURCE_TYPE_PHOTON:
                    fprintf(file_ptr_ph,"\n");
                break;

                case SOURCE_TYPE_ELECTRON:
                    fprintf(file_ptr_el,"\n");
                break;

                case SOURCE_TYPE_UNDEFINED:
                    fprintf(file_ptr_un,"\n");
                break;
            }
        }
    }

    // fprintf(file_time_stamp,"%f\n",ptr_to_lc_per_energy[index1].ph_el_un_arr[src_type]);
    
    fclose(file_ptr_el);
    fclose(file_ptr_ph);
    fclose(file_ptr_un);
    fclose(file_time_stamp);
}