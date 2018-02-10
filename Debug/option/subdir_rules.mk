################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
option/cc932.obj: ../option/cc932.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/djdharmik/workspace_v7/driverlib_empty_project" --include_path="C:/ti/ccsv7/ccs_base/arm/include/" --include_path="C:/ti/ccsv7/ccs_base/arm/include/CMSIS" --include_path="C:/ti/msp/MSP432Ware_3_50_00_02/driverlib/driverlib/MSP432P4xx" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/include/" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="option/cc932.d" --obj_directory="option" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


