################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
OxiSc_Modules/BattMonitor.obj: ../OxiSc_Modules/BattMonitor.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_17.9.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/david/Desktop/Project Files/OxiScope_MSP432_v1.14" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12/kernel/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_17.9.0.STS/include" --advice:power=all --advice:power_severity=suppress --define=ARM_MATH_CM4 --define=__MSP432P401R__ -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="OxiSc_Modules/BattMonitor.d" --obj_directory="OxiSc_Modules" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

OxiSc_Modules/CC2650_comm.obj: ../OxiSc_Modules/CC2650_comm.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_17.9.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/david/Desktop/Project Files/OxiScope_MSP432_v1.14" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12/kernel/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_17.9.0.STS/include" --advice:power=all --advice:power_severity=suppress --define=ARM_MATH_CM4 --define=__MSP432P401R__ -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="OxiSc_Modules/CC2650_comm.d" --obj_directory="OxiSc_Modules" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

OxiSc_Modules/OxiSc_Util.obj: ../OxiSc_Modules/OxiSc_Util.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_17.9.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/david/Desktop/Project Files/OxiScope_MSP432_v1.14" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12/kernel/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_17.9.0.STS/include" --advice:power=all --advice:power_severity=suppress --define=ARM_MATH_CM4 --define=__MSP432P401R__ -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="OxiSc_Modules/OxiSc_Util.d" --obj_directory="OxiSc_Modules" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

OxiSc_Modules/SpO2_PulseRate.obj: ../OxiSc_Modules/SpO2_PulseRate.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_17.9.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/david/Desktop/Project Files/OxiScope_MSP432_v1.14" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12/kernel/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_17.9.0.STS/include" --advice:power=all --advice:power_severity=suppress --define=ARM_MATH_CM4 --define=__MSP432P401R__ -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="OxiSc_Modules/SpO2_PulseRate.d" --obj_directory="OxiSc_Modules" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

OxiSc_Modules/UserInterface.obj: ../OxiSc_Modules/UserInterface.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_17.9.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/david/Desktop/Project Files/OxiScope_MSP432_v1.14" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12/kernel/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/simplelink_msp432p4_sdk_1_50_00_12" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_17.9.0.STS/include" --advice:power=all --advice:power_severity=suppress --define=ARM_MATH_CM4 --define=__MSP432P401R__ -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="OxiSc_Modules/UserInterface.d" --obj_directory="OxiSc_Modules" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


