import 	matplotlib.pyplot as plt
import 	numpy as np
from 	numpy import loadtxt

source_file_name = "ADC_ReadOut_"

num_of_elements = 26*10000

#Read from txt file, store as 2D arrays
HG_array = loadtxt(source_file_name + "HG.txt", unpack=False)
LG_array = loadtxt(source_file_name + "LG.txt", unpack=False)


def calculate_mean(file_array):

	return np.mean(file_array)

def calculate_std(file_array):

	return np.std(file_array)

def draw_random_normal_sample(mu, sigma, min_):

	array = np.random.normal(mu, sigma, num_of_elements+40000)
	return [num for num in array if num > min_]

def draw_random_poisson_sample(mu):
	return np.random.poisson(mu, num_of_elements)

def draw_random_exponential_sample(mu):
	return np.random.exponential(mu, num_of_elements)

def write_to_a_file(array, file_to_write):

	for index, num in enumerate(array):

		if (index % 26 == 0 and index != 0):
			file_to_write.write("\n")

		if (index <= num_of_elements):
			file_to_write.write(str(int(num)) + " ")


###########################################################
#################GAUSSIAN DISTRIBUTION#####################
###########################################################

# print("HG FILE")
# print("mean = ", calculate_mean(HG_array))
# print("std = ", calculate_std(HG_array))
# print("min = ", np.min(HG_array), "max = ",np.max(HG_array))

# print("LG FILE")
# print("mean = ", calculate_mean(LG_array))
# print("std = ", calculate_std(LG_array))
# print("min = ", np.min(LG_array), "max = ", np.max(LG_array))

# random_array_HG = draw_random_normal_sample(calculate_mean(HG_array), calculate_std(HG_array), 	300)
# random_array_LG = draw_random_normal_sample(calculate_mean(LG_array), calculate_std(LG_array)+200, 300)

# plt.hist(random_array_HG,40)
# plt.hist(random_array_LG,40)

# plt.show()

# file_random_HG = open(source_file_name + "random_HG.txt","w")
# file_random_LG = open(source_file_name + "random_LG.txt","w")

# write_to_a_file(random_array_HG,file_random_HG)
# write_to_a_file(random_array_LG,file_random_LG)

# file_random_HG.close()
# file_random_LG.close()


###########################################################
#################POISSON DISTRIBUTION######################
###########################################################


#plt.hist(draw_random_poisson_sample(calculate_mean(HG_array))+900,40)
#plt.hist(draw_random_poisson_sample(calculate_mean(LG_array))+1150,40)
#plt.show()

# random_array_HG = draw_random_poisson_sample(calculate_mean(HG_array)+900)
# random_array_LG = draw_random_poisson_sample(calculate_mean(LG_array)+1150)

# file_random_HG = open(source_file_name + "random_poisson_HG.txt","w")
# file_random_LG = open(source_file_name + "random_poisson_LG.txt","w")

# write_to_a_file(random_array_HG,file_random_HG)
# write_to_a_file(random_array_LG,file_random_LG)

# file_random_HG.close()
# file_random_LG.close()


###########################################################
#################EXPONENTTIAL DISTRIBUTION#################
###########################################################


# plt.hist(draw_random_exponential_sample(calculate_mean(HG_array))+900,40)
# plt.hist(draw_random_exponential_sample(calculate_mean(LG_array))+1150,40)
# plt.show()

random_array_HG = draw_random_exponential_sample(calculate_mean(HG_array)+900)
random_array_LG = draw_random_exponential_sample(calculate_mean(LG_array)+1150)

file_random_HG = open(source_file_name + "random_exponential_HG.txt","w")
file_random_LG = open(source_file_name + "random_exponential_LG.txt","w")

write_to_a_file(random_array_HG,file_random_HG)
write_to_a_file(random_array_LG,file_random_LG)

file_random_HG.close()
file_random_LG.close()