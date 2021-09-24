import os

#here provide the path to the folder that contains LG and HG files
source_path_name = "/home/marcinmust/Desktop/Infinity/School/Licence_Physique_UP7/L3/Internship/Other/sci_payload_measurments_data/Cobalt60"
os.chdir(source_path_name)

def prepare_test_data(target_file_name, source_file_name, start_idx, end_idx, LG_HG_flag):
	
	"""
	Description : 	prepares raw input test data
					(output data from test acquisitions
					with EASIROC board)

	Input: 	source_file_name - name of the file to analyze
			target_file_name - name of the output file
			start_idx 		 - start index of a file name prefix 
			end_idx			 - end 	 index of a file name prefix

								----------------
								START OF EXAMPLE
								----------------

								input files:
								
								ADC_ReadOut_HG_1001.lvm
								ADC_ReadOut_HG_11000.lvm

								start_idx 	= 1001
								end_idx 	= 11001

								----------------
								END OF EXAMPLE
								----------------

 			LG_HG_flag 		 -  if 1 LC_HG_flag acquires LG data
								if 0 LC_HG_flag acquires HG data

	Output: 		Outputs one .txt file per LG and HG data
	"""


	target_file  = open(target_file_name,'w')

	for source_file_num in range(start_idx,end_idx):

	    source_file = open(source_file_name + str(source_file_num) + ".lvm","r")

	    for index, row in enumerate(source_file):               
	        adc_value = row.split()[2]

	        if (LG_HG_flag == 1):
	        	if adc_value == "OTR_LG" or index > 26:
	        	    continue
	       	else:
	       		if adc_value == "OTR_HG" or index > 26:
	        	    continue

	        target_file.write(adc_value + ' ')
	    target_file.write('\n')

	    source_file.close()

	target_file.write('\0')

	target_file.close()

	if (LG_HG_flag == 1):
		print("LG Data preparation is finished successfully")
	else:
		print("HG Data preparation is finished successfully")

def main():

	start_idx = 1001
	end_idx = 11001
	
	prepare_test_data("ADC_ReadOut_LG.txt", "ADC_ReadOut_LG_", start_idx, end_idx, 1)
	prepare_test_data("ADC_ReadOut_HG.txt", "ADC_ReadOut_HG_", start_idx, end_idx, 0)
	print("Your output files are located in the following directory " + source_path_name)

if __name__ == '__main__':

    main()