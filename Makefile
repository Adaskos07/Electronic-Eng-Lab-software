CC                    := gcc
BIN                   := codesign

BUILD_DIR             := ./build
BUILD_DIR_ABS         := $(abspath $(BUILD_DIR))

KERNEL_DIR            := FreeRTOS-Kernel

TRACE_DIR             := FreeRTOS-Plus-Trace
MQTT_CORE_DIR         := coreMQTT-Agent/source/dependency/coreMQTT
MQTT_AGENT_DIR        := coreMQTT-Agent
TCP_DIR               := FreeRTOS-Plus-TCP

INCLUDE_DIRS          := -I.
INCLUDE_DIRS          += -I${KERNEL_DIR}/include
INCLUDE_DIRS          += -I${KERNEL_DIR}/portable/ThirdParty/GCC/Posix
INCLUDE_DIRS          += -I${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils
# INCLUDE_DIRS          += -I${MQTT_CORE_DIR}/source/include
# INCLUDE_DIRS          += -I${MQTT_CORE_DIR}/source/interface
# INCLUDE_DIRS          += -I${MQTT_AGENT_DIR}/source/include

INCLUDE_DIRS          += -I${TCP_DIR}/portable/NetworkInterface/linux/
INCLUDE_DIRS          += -I${TCP_DIR}/include/
INCLUDE_DIRS          += -I${TCP_DIR}/portable/Compiler/GCC/

# INCLUDE_DIRS          += -I${TRACE_DIR}/Include

SOURCE_FILES          := $(wildcard *.c)
SOURCE_FILES          += $(wildcard ${KERNEL_DIR}/*.c)
# Memory manager (use malloc() / free() )
SOURCE_FILES          += ${KERNEL_DIR}/portable/MemMang/heap_3.c
# posix port
SOURCE_FILES          += ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
SOURCE_FILES          += ${KERNEL_DIR}/portable/ThirdParty/GCC/Posix/port.c
# core mqtt
# SOURCE_FILES          += ${MQTT_CORE_DIR}/source/core_mqtt_serializer.c
# SOURCE_FILES          += ${MQTT_CORE_DIR}/source/core_mqtt_state.c
# SOURCE_FILES          += ${MQTT_CORE_DIR}/source/core_mqtt.c
# mqtt agent
# SOURCE_FILES          += ${MQTT_AGENT_DIR}/source/core_mqtt_agent_command_functions.c
# SOURCE_FILES          += ${MQTT_AGENT_DIR}/source/core_mqtt_agent.c
# FreeRTOS TCP
SOURCE_FILES          += ${TCP_DIR}/FreeRTOS_DNS.c
SOURCE_FILES          += ${TCP_DIR}/FreeRTOS_DHCP.c
SOURCE_FILES          += ${TCP_DIR}/FreeRTOS_ARP.c
SOURCE_FILES          += ${TCP_DIR}/FreeRTOS_TCP_WIN.c
SOURCE_FILES          += ${TCP_DIR}/FreeRTOS_Stream_Buffer.c
SOURCE_FILES          += ${TCP_DIR}/portable/BufferManagement/BufferAllocation_2.c
SOURCE_FILES          += ${TCP_DIR}/FreeRTOS_IP.c
SOURCE_FILES          += ${TCP_DIR}/FreeRTOS_TCP_IP.c
SOURCE_FILES          += ${TCP_DIR}/FreeRTOS_UDP_IP.c
SOURCE_FILES          += ${TCP_DIR}/FreeRTOS_Sockets.c
SOURCE_FILES          += ${TCP_DIR}/portable/NetworkInterface/linux/NetworkInterface.c





# # Trace library.
# SOURCE_FILES          += ${TRACE_DIR}/trcKernelPort.c
# SOURCE_FILES          += ${TRACE_DIR}/trcSnapshotRecorder.c
# SOURCE_FILES          += ${TRACE_DIR}/trcStreamingRecorder.c
# SOURCE_FILES          += ${TRACE_DIR}/streamports/File/trcStreamingPort.c


CFLAGS                :=    -ggdb3 -DprojCOVERAGE_TEST=0 -D_WINDOWS_
LDFLAGS               :=    -ggdb3 -pthread -lpcap
CPPFLAGS              :=    $(INCLUDE_DIRS) -DBUILD_DIR=\"$(BUILD_DIR_ABS)\"

ifdef PROFILE
  CFLAGS              +=   -pg  -O0
  LDFLAGS             +=   -pg  -O0
else
  CFLAGS              +=   -O3
  LDFLAGS             +=   -O3
endif

ifdef SANITIZE_ADDRESS
  CFLAGS              +=   -fsanitize=address -fsanitize=alignment
  LDFLAGS            +=   -fsanitize=address -fsanitize=alignment
endif

ifdef SANITIZE_LEAK
  LDFLAGS            +=   -fsanitize=leak
endif


OBJ_FILES = $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.o)

DEP_FILE = $(OBJ_FILES:%.o=%.d)

${BIN} : $(BUILD_DIR)/$(BIN)

${BUILD_DIR}/${BIN} : ${OBJ_FILES}
	-mkdir -p ${@D}
	$(CC) $^ ${LDFLAGS} -o $@

-include ${DEP_FILE}

${BUILD_DIR}/%.o : %.c Makefile
	-mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -c $< -o $@

.PHONY: clean

clean:
	-rm -rf $(BUILD_DIR)


GPROF_OPTIONS := --directory-path=$(INCLUDE_DIRS)
profile:
	gprof -a -p --all-lines $(GPROF_OPTIONS) $(BUILD_DIR)/$(BIN) $(BUILD_DIR)/gmon.out > $(BUILD_DIR)/prof_flat.txt
	gprof -a --graph $(GPROF_OPTIONS) $(BUILD_DIR)/$(BIN) $(BUILD_DIR)/gmon.out > $(BUILD_DIR)/prof_call_graph.txt






