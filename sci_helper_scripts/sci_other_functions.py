import matplotlib.pyplot as plt
import numpy as np

# source_file_name = "../output/sci_light_output_"

def plot_2d_spectrum_histogram(spectrum_type_arr):
    
    """
    plots histograms of a energy as function of
    N counts of a given energy

    :spectrum_type_arr: a list of len 2, containing 
    electron, photon and undefined spectrums for a one
    batch of the measurment

    :return: void
    """
    for index, spectrum_type in enumerate(spectrum_type_arr):

        # print(spectrum_type)
        if (spectrum_type != []):
            #bins = int((max(spectrum_type) - min(spectrum_type))/20)
            #print("bins = ", bins)

            #if ( bins > 0 ):

            plt.hist(spectrum_type,20)

            if (index == 0):
                plt.title("Electron")
            elif (index == 1):
                plt.title("Photon")
            else:
                plt.title("Undefined")

            plt.xlabel('Energy')
            plt.show()


def plot_3d_histogram(spectrum_source_type_batch,N_time_bins):

    """
    plots histograms of a energy as function of
    N counts and time

    :spectrum_source_type_batch: a lsit of size Nxm
    N - number of batches
    m - nubmer of valid entries

    :return: void
    """

    time_array = np.linspace(0,1,N_time_bins)

    fig = plt.figure()
    ax = plt.axes(projection="3d")
    for batch in spectrum_source_type_batch:

        _time_array, _spectrum_electron_per_bach = np.meshgrid(time_array,batch)
        x, y = _time_array.ravel(), _spectrum_electron_per_bach.ravel()

        top = x + y
        bottom = np.zeros_like(top)
        width = depth = 1

        ax.bar3d(x, y,bottom,width,depth,top,shade=True)
        
        ax.set_xlabel("Time")
        ax.set_ylabel("Energy")
        ax.set_zlabel("N")

        plt.show()


def prepare_spectrum_data_2d_histograms(source_file_name):
    
    """
    prepares energy data as function of N for
    a plot_energy_function_of_n_histograms()
    function
    
    :source_file_name: path to the source file

    :return: returns 3 lists for electron, photon,
    and undefined spectrums
    """

    source_file = open(source_file_name,"r")

    spectrum_electron = []
    spectrum_photon = []
    spectrum_undefined = []

    for row in source_file:

        if (row != '\n'):

            energy_string_value = row.split()
            
            electron_adc_value = int(energy_string_value[0])
            photon_adc_value = int(energy_string_value[1])
            undefined_adc_value = int(energy_string_value[2])
            
            if (electron_adc_value != 0):
                spectrum_electron.append(electron_adc_value)
            if (photon_adc_value != 0):
                spectrum_photon.append(photon_adc_value)
            if (undefined_adc_value != 0):
                spectrum_undefined.append(undefined_adc_value)

    source_file.close()

    return [spectrum_electron,spectrum_photon,spectrum_undefined]


def prepare_data_3d_histograms(source_file_name):

    """
    prepares data for a 3d histograms 3d (E,N,t)
    
    :source_file_name: path to the source file

    :return: returns spectrum_source_type_per_batch array
    containing N energy values for N acquisitions
    """

    source_file = open(source_file_name,"r")

    spectrum_electron = []
    spectrum_photon = []
    spectrum_undefined = []

    spectrum_electron_per_bach = []
    
    for row in source_file:
           
        energy_string_value = row.split()

        if (energy_string_value == []):
            spectrum_electron_per_bach.append(spectrum_electron)
            spectrum_electron = []
            continue
        
        else:
            electron_adc_value = int(energy_string_value[2])        
            if (electron_adc_value != 0):
                spectrum_electron.append(electron_adc_value)
            
        # photon_adc_value = int(row.split()[1])
        # undefined_adc_value = int(row.split()[2])
        
        # if (photon_adc_value != 0):
            # spectrum_photon.append(photon_adc_value)
        #if (undefined_adc_value != 0):
            # spectrum_undefined.append(undefined_adc_value)

    source_file.close()

    return spectrum_electron_per_bach


def plot_test_data():

    source_file = open("ADC_ReadOut_LG.txt","r")

    test_array = []
    channel_energy_sum = 0
    for row in source_file:

        adc_value = row.split()
        channel_energy_sum = 0
        
        for index, value in enumerate(adc_value):
            value = int(value)
            if (value < 900 and index > 15):
                continue
            channel_energy_sum += value
        
        test_array.append(channel_energy_sum)

    bins = int((max(test_array) - min(test_array))/15)
    print(bins)
    plt.hist(test_array, bins = bins)
    plt.show()
