MotatePathBase ?= ./
MotatePathBase := $(MotatePathBase)'Motate/Arduino Library/'

MotateIncludes := -I $(MotatePathBase)Motate/

MotateLCDIncludes := -I $(MotatePathBase)MotateLCD/

MotateTWIIncludes := -I $(MotatePathBase)MotateTWI/
