import matplotlib.pyplot as plt
import numpy as np
import math

def data_preparation(data_type):
    
    """
    Description: Prepares SP & LC output data for plotting

    Input: data_type : "SP" - prepare spectrum, "LC" - prepare light curves

    Return: particle_types_array of length 2, containing prepared
                2D data for electron, photon and undefined particle
    """
        
    source_directory = "../output/SP/" if data_type == "SP" else "../output/LC/"
    particle_types   = ['photon','electron','undefined']

    particle_types_array = []     
    for particle_type in particle_types: 
        
        data_array  = []
        source_file = open(source_directory + particle_type + '.txt',"r")
        data_array  = [int(row) for row in source_file if row != '\n']
        particle_types_array.append(data_array)
        source_file.close()

    return particle_types_array


def prepare_lc_per_energy_data():

    """
    Description: Prepares light curves per energy find at (output/LC_PER_ENERGY)
                 data for plotting.

    Input:  void

    Return: src_output_arr - Is an array containing compressed lc data
                             for electrons, photons and undefined particles.
    """

    source_directory = "../output/LC_PER_ENERGY/"

    particle_types      = ['photon','electron','undefined']
    
    PH_LC_per_EN = []
    EL_LC_per_EN = []
    UN_LC_per_EN = []
    
    for index, part_type in enumerate(particle_types):
        
        src_file = open(source_directory + part_type + '.txt',"r")

        LC_per_energy_arr = []
        num_counts_per_dt = []
        num_consec_zeros  = []

        for row in src_file:

            if (row == "\n"):
                LC_per_energy_arr.append(num_counts_per_dt)
                LC_per_energy_arr.append(num_consec_zeros)
                
                num_counts_per_dt = []
                num_consec_zeros  = []
                
            else:
                string = row.split()
                num_counts_per_dt.append(int(string[0]))
                num_consec_zeros.append(int(string[1]))
        
        if   (index == 0):
            PH_LC_per_EN.append(LC_per_energy_arr)
        elif (index == 1):
            EL_LC_per_EN.append(LC_per_energy_arr)
        else:
            UN_LC_per_EN.append(LC_per_energy_arr)

    return PH_LC_per_EN, EL_LC_per_EN, UN_LC_per_EN 


def reconstruct_compressed_lc_data(SRC_LC_per_EN):

    """
    Description: Reconstructs light curves from the compressed data

    Input: SRC_LC_per_EN -      particle type object array - output array
                                from the prepare_lc_per_energy_data()
                                function for a given particle type

    Return: src_output_array -  light curves per energy array, for a given
                                particle, ready to be plotted
    """

    src_output_array = []

    #loop over array containining number of particle
    #counts and number of zeros for each energy
    #for a given particle type
    for i in range(0, len(SRC_LC_per_EN[0])):

        #particles_per_dt and number of zeros
        #arrays are stacked next to each other
        reconstr_arr = []
        if (i % 2 == 0):

            num_counts = SRC_LC_per_EN[0][i]
            num_zeros  = SRC_LC_per_EN[0][i+1]

            #for each particle in particles per dt
            #and number of zeros arrays
            for j in range(0, len(num_counts)):

                #if number of zeros == 0
                #a given number 
                #of particles of a given energy was detected,
                #append this number of particles

                #else
                #a particle of a given energy, at a given
                #time stamp wasn't detected therefore append 0
                if(num_zeros[j] == 0):
                    reconstr_arr.append(num_counts[j])
                else:
                    for k in range(0, num_zeros[j]):
                        reconstr_arr.append(0)
                    reconstr_arr.append(num_counts[j])

            src_output_array.append(reconstr_arr)

    return src_output_array

def get_src_type(src_type):

    if (src_type == 0):
        return "Photon"
    elif (src_type == 1):
        return "Electron"
    else:
        return "Undefined" 


def plot_lc(src_reconstr_arr,energy_array,src_type):

    """
    Description : Plots light curves array for a given energy

    Input : src_reconstr_arr - reconstructed soruce array
            reconstruct_compressed_lc_data() function output

            energy array  - Energy array specified in the speactrum.c file,
                            for which light curves per energy where generated.

            src_type      - source for which plots are made, should be the same as     
                            SRC_LC_per_EN
                            0 - photon
                            1 - electron
                            2 - undefined

    """

    fontsize = 35
    string = " light curves per Energy bin "

    for i, energy in enumerate(src_reconstr_arr):

        time_step = get_time_step_per_lc(energy_array[i],src_type)

        energy[:] = [counts/time_step for counts in energy]
        
        plt.plot(np.arange(len(energy))*time_step, energy, '*')
        plt.xlabel('Number of bins x Δt', fontsize = fontsize)
        plt.ylabel('Counts/Δt',fontsize = fontsize)
        plt.ylim(0,0.6)
        plt.title(get_src_type(src_type) + string + str(energy_array[i]), fontsize = fontsize)

        plt.xticks(fontsize=fontsize)
        plt.yticks(fontsize=fontsize)
        plt.show()

def plot_histogram(particle_types_array, histogram_type):

    """
    Description: Plots spectrum histograms and light curves.
                 Preapre the data first using data_preparation(),
                 then call the plot_histogram()

    Input:       particle_types_array - output of the data_preparation()
                 histogram_type - "LC" or "SP"

    Ouputs:      void
    """
    fontsize = 35
    particle_types = ['photon','electron', 'undefined']
    string = " histogram, particle type :"

    for index1, particle_array in enumerate(particle_types_array):

        if (particle_array != []):
                
            if (histogram_type == "LC"):

                particle_array = np.array(particle_array)
                
                time_step1, time_step2 = get_time_step()

                particle_array[:-1] = particle_array[:-1]/time_step1
                #last particles are regrouped with a different time_step
                #than all the other particles
                particle_array[-1]  = particle_array[-1]/time_step2
                
                plt.plot(np.arange(len(particle_array))*time_step1,particle_array, '.')
                plt.xlabel('Number of bins x Δt', fontsize = fontsize)
                plt.ylabel('Counts/Δt',fontsize = fontsize)
                plt.ylim(0,100)
                plt.title(histogram_type + string + particle_types[index1] ,fontsize = fontsize)
            
            else:

                plt.bar(np.arange(len(particle_array)),particle_array)
                plt.xlabel('Particle Energy bins, arbitrary units', fontsize = fontsize)
                plt.ylabel('Number of counts',fontsize = fontsize)      
                plt.title(histogram_type + string + particle_types[index1], fontsize = fontsize)

            plt.xticks(fontsize=fontsize)
            plt.yticks(fontsize=fontsize)
            plt.show()


def get_time_step():

    """
    Description : gets the time steps for light curves of total energies    
    """

    time_step = open("../output/LC/" + "time_stamp" + '.txt',"r")
    time_step_arr = np.array(time_step.readlines())
    time_step1 = float(time_step_arr[0])
    time_step2 = float(time_step_arr[1])
    time_step.close()

    return time_step1, time_step2

def get_time_step_per_lc(energy, src_type):

    time_src = open("../output/LC_PER_ENERGY/" + "time_stamp" + '.txt',"r")

    for time_step in time_src:

        string = time_step.split()
        if (int(string[1]) == energy and int(string[2]) == src_type):
            return float(string[0])


def plot_light_curves():
    
    """
    Description : Plots the light curves - evolution
    of all energies as a function of time index

    Input: void

    return: void
    """

    plot_histogram(data_preparation("LC"), "LC")


def plot_spectrum_histograms():
    
    """
    Description : Plots the spectrum histograms - distribution
    of N counts as a function of energy indices

    Input: void
    
    return: void
    """

    plot_histogram(data_preparation("SP"), "SP")

def plot_lc_per_energy(energy_array, SRC_LC_per_EN, src_type):

    """
    Description: The function plots light curves per energy bin
    
    Input:  energy array  - Energy array specified in the speactrum.c file,
                            for which light curves per energy where generated.

            SRC_LC_per_EN - Output of the prepare_lc_per_energy_data function
                            for a given energy

            src_type      - source for which plots are made, should be the same as     
                            SRC_LC_per_EN
                            0 - photon
                            1 - electron
                            2 - undefined

    return: void
    """

    if (src_type == 0):
        ph_reconstr_arr = reconstruct_compressed_lc_data(SRC_LC_per_EN)
        plot_lc(ph_reconstr_arr, energy_array, 0)
    elif (src_type == 1):
        el_reconstr_arr = reconstruct_compressed_lc_data(SRC_LC_per_EN)
        plot_lc(el_reconstr_arr, energy_array, 1)
    else:
        un_reconstr_arr = reconstruct_compressed_lc_data(SRC_LC_per_EN)
        plot_lc(un_reconstr_arr, energy_array, 2)


def main():

    #Uncommennt the code between the lines below to 
    #plot light curves and specturm historgrams
    #---------------------------------------------------
    # plot_spectrum_histograms()
    # plot_light_curves()
    #---------------------------------------------------

    #Uncommennt the code between the lines below to 
    #plot light curves per given energy, specify 
    #energy_array - array of energies for which lc's per
    #energy were calculated in the spectrum.c function
    #You can change the zoom of the plot in the plot_lc
    #function by modifying ylim(min,max)
    #---------------------------------------------------
    energy_array = [7,8,9,10,11]
    PH_LC_per_EN, EL_LC_per_EN, UN_LC_per_EN = prepare_lc_per_energy_data()
    plot_lc_per_energy(energy_array, UN_LC_per_EN, 2)    
    #---------------------------------------------------

if __name__ == '__main__':

    main()