################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../boinc/boinc/coverity-model.cpp 

OBJS += \
./boinc/boinc/coverity-model.o 

CPP_DEPS += \
./boinc/boinc/coverity-model.d 


# Each subdirectory must supply rules for building sources it contributes
boinc/boinc/%.o: ../boinc/boinc/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O2 -g -Wall -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


