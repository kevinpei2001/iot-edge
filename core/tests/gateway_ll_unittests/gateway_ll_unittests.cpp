// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <cstdlib>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "testrunnerswitcher.h"
#include "micromock.h"
#include "micromockcharstararenullterminatedstrings.h"
#include "azure_c_shared_utility/lock.h"

#include "gateway_ll.h"
#include "broker.h"
#include "internal/event_system.h"
#include "module_loader.h"

#define DUMMY_LIBRARY_PATH "x.dll"

#define GBALLOC_H

DEFINE_MICROMOCK_ENUM_TO_STRING(GATEWAY_ADD_LINK_RESULT, GATEWAY_ADD_LINK_RESULT_VALUES);

extern "C" int gballoc_init(void);
extern "C" void gballoc_deinit(void);
extern "C" void* gballoc_malloc(size_t size);
extern "C" void* gballoc_calloc(size_t nmemb, size_t size);
extern "C" void* gballoc_realloc(void* ptr, size_t size);
extern "C" void gballoc_free(void* ptr);

namespace BASEIMPLEMENTATION
{
	/*if malloc is defined as gballoc_malloc at this moment, there'd be serious trouble*/

#define Lock(x) (LOCK_OK + gballocState - gballocState) /*compiler warning about constant in if condition*/
#define Unlock(x) (LOCK_OK + gballocState - gballocState)
#define Lock_Init() (LOCK_HANDLE)0x42
#define Lock_Deinit(x) (LOCK_OK + gballocState - gballocState)
#include "gballoc.c"
#undef Lock
#undef Unlock
#undef Lock_Init
#undef Lock_Deinit
#include "vector.c"

};

static size_t currentmalloc_call;
static size_t whenShallmalloc_fail;

static size_t currentBroker_AddModule_call;
static size_t whenShallBroker_AddModule_fail;
static size_t currentBroker_RemoveModule_call;
static size_t whenShallBroker_RemoveModule_fail;
static size_t currentBroker_Create_call;
static size_t whenShallBroker_Create_fail;
static size_t currentBroker_module_count;
static size_t currentBroker_ref_count;

static size_t currentModuleLoader_Load_call;
static size_t whenShallModuleLoader_Load_fail;

static size_t currentModule_Create_call;
static size_t currentModule_Destroy_call;

static size_t currentVECTOR_create_call;
static size_t whenShallVECTOR_create_fail;
static size_t currentVECTOR_push_back_call;
static size_t whenShallVECTOR_push_back_fail;
static size_t currentVECTOR_find_if_call;
static size_t whenShallVECTOR_find_if_fail;

static MODULE_APIS dummyAPIs;

TYPED_MOCK_CLASS(CGatewayLLMocks, CGlobalMock)
{
public:
	MOCK_STATIC_METHOD_2(, MODULE_HANDLE, mock_Module_Create, BROKER_HANDLE, broker, const void*, configuration)
		currentModule_Create_call++;
		MODULE_HANDLE result1;
		if (configuration != NULL && *((bool*)configuration) == false)
		{
			result1 = NULL;
		}
		else
		{
			result1 = (MODULE_HANDLE)BASEIMPLEMENTATION::gballoc_malloc(currentModule_Create_call);
		}
	MOCK_METHOD_END(MODULE_HANDLE, result1);

	MOCK_STATIC_METHOD_1(, void, mock_Module_Destroy, MODULE_HANDLE, moduleHandle)
		currentModule_Destroy_call++;
		BASEIMPLEMENTATION::gballoc_free(moduleHandle);
	MOCK_VOID_METHOD_END();

	MOCK_STATIC_METHOD_2(, void, mock_Module_Receive, MODULE_HANDLE, moduleHandle, MESSAGE_HANDLE, messageHandle)
	MOCK_VOID_METHOD_END();

	MOCK_STATIC_METHOD_1(, void, Broker_DecRef, BROKER_HANDLE, broker)
		if (currentBroker_ref_count > 0)
		{
			--currentBroker_ref_count;
			if (currentBroker_ref_count == 0)
			{
				BASEIMPLEMENTATION::gballoc_free(broker);
			}
		}
	MOCK_VOID_METHOD_END();

	MOCK_STATIC_METHOD_1(, void, Broker_IncRef, BROKER_HANDLE, broker)
		++currentBroker_ref_count;
	MOCK_VOID_METHOD_END();

	MOCK_STATIC_METHOD_0(, BROKER_HANDLE, Broker_Create)
	BROKER_HANDLE result1;
	currentBroker_Create_call++;
	if (whenShallBroker_Create_fail >= 0 && whenShallBroker_Create_fail == currentBroker_Create_call)
	{
		result1 = NULL;
	}
	else
	{
		++currentBroker_ref_count;
		result1 = (BROKER_HANDLE)BASEIMPLEMENTATION::gballoc_malloc(1);
	}
	MOCK_METHOD_END(BROKER_HANDLE, result1);

	MOCK_STATIC_METHOD_1(, void, Broker_Destroy, BROKER_HANDLE, broker)
		if (currentBroker_ref_count > 0)
		{
			--currentBroker_ref_count;
			if (currentBroker_ref_count == 0)
			{
				BASEIMPLEMENTATION::gballoc_free(broker);
			}
		}
	MOCK_VOID_METHOD_END();

	MOCK_STATIC_METHOD_2(, BROKER_RESULT, Broker_AddModule, BROKER_HANDLE, handle, const MODULE*, module)
		currentBroker_AddModule_call++;
		BROKER_RESULT result1  = BROKER_ERROR;
		if (handle != NULL && module != NULL)
		{
			if (whenShallBroker_AddModule_fail != currentBroker_AddModule_call)
			{
				++currentBroker_module_count;
				result1 = BROKER_OK;
			}
		}
	MOCK_METHOD_END(BROKER_RESULT, result1);

	MOCK_STATIC_METHOD_2(, BROKER_RESULT, Broker_RemoveModule, BROKER_HANDLE, handle, const MODULE*, module)
		currentBroker_RemoveModule_call++;
		BROKER_RESULT result1 = BROKER_ERROR;
		if (handle != NULL && module != NULL && currentBroker_module_count > 0 && whenShallBroker_RemoveModule_fail != currentBroker_RemoveModule_call)
		{
			--currentBroker_module_count;
			result1 = BROKER_OK;
		}
	MOCK_METHOD_END(BROKER_RESULT, result1);

	MOCK_STATIC_METHOD_1(, MODULE_LIBRARY_HANDLE, ModuleLoader_Load, const char*, moduleLibraryFileName)
		currentModuleLoader_Load_call++;
		MODULE_LIBRARY_HANDLE handle = NULL;
		if (whenShallModuleLoader_Load_fail >= 0 && whenShallModuleLoader_Load_fail != currentModuleLoader_Load_call)
		{
			handle = (MODULE_LIBRARY_HANDLE)BASEIMPLEMENTATION::gballoc_malloc(1);
		}
	MOCK_METHOD_END(MODULE_LIBRARY_HANDLE, handle);

	MOCK_STATIC_METHOD_1(, const MODULE_APIS*, ModuleLoader_GetModuleAPIs, MODULE_LIBRARY_HANDLE, module_library_handle)
		const MODULE_APIS* apis = &dummyAPIs;
	MOCK_METHOD_END(const MODULE_APIS*, apis);

	MOCK_STATIC_METHOD_1(, void, ModuleLoader_Unload, MODULE_LIBRARY_HANDLE, moduleLibraryHandle)
		BASEIMPLEMENTATION::gballoc_free(moduleLibraryHandle);
	MOCK_VOID_METHOD_END();

	MOCK_STATIC_METHOD_0(, EVENTSYSTEM_HANDLE, EventSystem_Init)
	MOCK_METHOD_END(EVENTSYSTEM_HANDLE, (EVENTSYSTEM_HANDLE)BASEIMPLEMENTATION::gballoc_malloc(1));

	MOCK_STATIC_METHOD_3(, void, EventSystem_AddEventCallback, EVENTSYSTEM_HANDLE, event_system, GATEWAY_EVENT, event_type, GATEWAY_CALLBACK, callback)
		// no-op
	MOCK_VOID_METHOD_END();

	MOCK_STATIC_METHOD_3(, void, EventSystem_ReportEvent, EVENTSYSTEM_HANDLE, event_system, GATEWAY_HANDLE, gw, GATEWAY_EVENT, event_type)
		// no-op
	MOCK_VOID_METHOD_END();

	MOCK_STATIC_METHOD_1(, void, EventSystem_Destroy, EVENTSYSTEM_HANDLE, handle)
		BASEIMPLEMENTATION::gballoc_free(handle);
	MOCK_VOID_METHOD_END();

	MOCK_STATIC_METHOD_1(, VECTOR_HANDLE, VECTOR_create, size_t, elementSize)
		currentVECTOR_create_call++;
		VECTOR_HANDLE vector = NULL;
		if (whenShallVECTOR_create_fail != currentVECTOR_create_call)
		{
			vector = BASEIMPLEMENTATION::VECTOR_create(elementSize);
		}
	MOCK_METHOD_END(VECTOR_HANDLE, vector);

	MOCK_STATIC_METHOD_1(, void, VECTOR_destroy, VECTOR_HANDLE, handle)
		BASEIMPLEMENTATION::VECTOR_destroy(handle);
	MOCK_VOID_METHOD_END();

	MOCK_STATIC_METHOD_3(, int, VECTOR_push_back, VECTOR_HANDLE, handle, const void*, elements, size_t, numElements)
		currentVECTOR_push_back_call++;
		int result1 = -1;
		if (whenShallVECTOR_push_back_fail != currentVECTOR_push_back_call)
		{
			result1 = BASEIMPLEMENTATION::VECTOR_push_back(handle, elements, numElements);
		}
	MOCK_METHOD_END(int, result1);

	MOCK_STATIC_METHOD_3(, void, VECTOR_erase, VECTOR_HANDLE, handle, void*, elements, size_t, index)
		BASEIMPLEMENTATION::VECTOR_erase(handle, elements, index);
	MOCK_VOID_METHOD_END();

	MOCK_STATIC_METHOD_2(, void*, VECTOR_element, const VECTOR_HANDLE, handle, size_t, index)
		auto element = BASEIMPLEMENTATION::VECTOR_element(handle, index);
	MOCK_METHOD_END(void*, element);

	MOCK_STATIC_METHOD_1(, void*, VECTOR_front, const VECTOR_HANDLE, handle)
		auto element = BASEIMPLEMENTATION::VECTOR_front(handle);
	MOCK_METHOD_END(void*, element);

	MOCK_STATIC_METHOD_1(, size_t, VECTOR_size, const VECTOR_HANDLE, handle)
		auto size = BASEIMPLEMENTATION::VECTOR_size(handle);
	MOCK_METHOD_END(size_t, size);

	MOCK_STATIC_METHOD_3(, void*, VECTOR_find_if, const VECTOR_HANDLE, handle, PREDICATE_FUNCTION, pred, const void*, value)
		currentVECTOR_find_if_call++;
		void* element = NULL;
		if (whenShallVECTOR_find_if_fail != currentVECTOR_find_if_call)
		{
			element = BASEIMPLEMENTATION::VECTOR_find_if(handle, pred, value);
		}
	MOCK_METHOD_END(void*, element);

	MOCK_STATIC_METHOD_1(, void*, gballoc_malloc, size_t, size)
		void* result2;
		currentmalloc_call++;
		if (whenShallmalloc_fail>0)
		{
			if (currentmalloc_call == whenShallmalloc_fail)
			{
				result2 = NULL;
			}
			else
			{
				result2 = BASEIMPLEMENTATION::gballoc_malloc(size);
			}
		}
		else
		{
			result2 = BASEIMPLEMENTATION::gballoc_malloc(size);
		}
	MOCK_METHOD_END(void*, result2);

	MOCK_STATIC_METHOD_2(, void*, gballoc_realloc, void*, ptr, size_t, size)
	MOCK_METHOD_END(void*, BASEIMPLEMENTATION::gballoc_realloc(ptr, size));

	MOCK_STATIC_METHOD_1(, void, gballoc_free, void*, ptr)
		BASEIMPLEMENTATION::gballoc_free(ptr);
	MOCK_VOID_METHOD_END()

};

DECLARE_GLOBAL_MOCK_METHOD_2(CGatewayLLMocks, , MODULE_HANDLE, mock_Module_Create, BROKER_HANDLE, broker, const void*, configuration);
DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , void, mock_Module_Destroy, MODULE_HANDLE, moduleHandle);
DECLARE_GLOBAL_MOCK_METHOD_2(CGatewayLLMocks, , void, mock_Module_Receive, MODULE_HANDLE, moduleHandle, MESSAGE_HANDLE, messageHandle);

DECLARE_GLOBAL_MOCK_METHOD_0(CGatewayLLMocks, , BROKER_HANDLE, Broker_Create);
DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , void, Broker_Destroy, BROKER_HANDLE, broker);
DECLARE_GLOBAL_MOCK_METHOD_2(CGatewayLLMocks, , BROKER_RESULT, Broker_AddModule, BROKER_HANDLE, handle, const MODULE*, module);
DECLARE_GLOBAL_MOCK_METHOD_2(CGatewayLLMocks, , BROKER_RESULT, Broker_RemoveModule, BROKER_HANDLE, handle, const MODULE*, module);
DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , void, Broker_IncRef, BROKER_HANDLE, broker);
DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , void, Broker_DecRef, BROKER_HANDLE, broker);

DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , MODULE_LIBRARY_HANDLE, ModuleLoader_Load, const char*, moduleLibraryFileName);
DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , const MODULE_APIS*, ModuleLoader_GetModuleAPIs, MODULE_LIBRARY_HANDLE, module_library_handle);
DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , void, ModuleLoader_Unload, MODULE_LIBRARY_HANDLE, moduleLibraryHandle);

DECLARE_GLOBAL_MOCK_METHOD_0(CGatewayLLMocks, , EVENTSYSTEM_HANDLE, EventSystem_Init);
DECLARE_GLOBAL_MOCK_METHOD_3(CGatewayLLMocks, , void, EventSystem_AddEventCallback, EVENTSYSTEM_HANDLE, event_system, GATEWAY_EVENT, event_type, GATEWAY_CALLBACK, callback);
DECLARE_GLOBAL_MOCK_METHOD_3(CGatewayLLMocks, , void, EventSystem_ReportEvent, EVENTSYSTEM_HANDLE, event_system, GATEWAY_HANDLE, gw, GATEWAY_EVENT, event_type);
DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , void, EventSystem_Destroy, EVENTSYSTEM_HANDLE, handle);

DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , VECTOR_HANDLE, VECTOR_create, size_t, elementSize);
DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , void, VECTOR_destroy, VECTOR_HANDLE, handle);
DECLARE_GLOBAL_MOCK_METHOD_3(CGatewayLLMocks, , int, VECTOR_push_back, VECTOR_HANDLE, handle, const void*, elements, size_t, numElements);
DECLARE_GLOBAL_MOCK_METHOD_3(CGatewayLLMocks, , void, VECTOR_erase, VECTOR_HANDLE, handle, void*, elements, size_t, index);
DECLARE_GLOBAL_MOCK_METHOD_2(CGatewayLLMocks, , void*, VECTOR_element, const VECTOR_HANDLE, handle, size_t, index);
DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , void*, VECTOR_front, const VECTOR_HANDLE, handle);
DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , size_t, VECTOR_size, const VECTOR_HANDLE, handle);
DECLARE_GLOBAL_MOCK_METHOD_3(CGatewayLLMocks, , void*, VECTOR_find_if, const VECTOR_HANDLE, handle, PREDICATE_FUNCTION, pred, const void*, value);


DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , void*, gballoc_malloc, size_t, size);
DECLARE_GLOBAL_MOCK_METHOD_2(CGatewayLLMocks, , void*, gballoc_realloc, void*, ptr, size_t, size);
DECLARE_GLOBAL_MOCK_METHOD_1(CGatewayLLMocks, , void, gballoc_free, void*, ptr)

static MICROMOCK_GLOBAL_SEMAPHORE_HANDLE g_dllByDll;
static MICROMOCK_MUTEX_HANDLE g_testByTest;

static GATEWAY_PROPERTIES* dummyProps;

static int sampleCallbackFuncCallCount;

static void expectEventSystemInit(CGatewayLLMocks &mocks)
{
	STRICT_EXPECTED_CALL(mocks, EventSystem_Init());
	STRICT_EXPECTED_CALL(mocks, EventSystem_ReportEvent(IGNORED_PTR_ARG, IGNORED_PTR_ARG, GATEWAY_CREATED))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, EventSystem_ReportEvent(IGNORED_PTR_ARG, IGNORED_PTR_ARG, GATEWAY_MODULE_LIST_CHANGED))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
}

static void expectEventSystemDestroy(CGatewayLLMocks &mocks)
{
	STRICT_EXPECTED_CALL(mocks, EventSystem_ReportEvent(IGNORED_PTR_ARG, IGNORED_PTR_ARG, GATEWAY_DESTROYED))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, EventSystem_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
}

static void sampleCallbackFunc(GATEWAY_HANDLE gw, GATEWAY_EVENT event_type, GATEWAY_EVENT_CTX ctx)
{
	sampleCallbackFuncCallCount++;
}

BEGIN_TEST_SUITE(gateway_ll_unittests)

TEST_SUITE_INITIALIZE(TestClassInitialize)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
	g_testByTest = MicroMockCreateMutex();
	ASSERT_IS_NOT_NULL(g_testByTest);
}

TEST_SUITE_CLEANUP(TestClassCleanup)
{
	MicroMockDestroyMutex(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
	if (!MicroMockAcquireMutex(g_testByTest))
	{
		ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
	}
	currentmalloc_call = 0;
	whenShallmalloc_fail = 0;

	currentBroker_AddModule_call = 0;
	whenShallBroker_AddModule_fail = 0;
	currentBroker_RemoveModule_call = 0;
	whenShallBroker_RemoveModule_fail = 0;
	currentBroker_Create_call = 0;
	whenShallBroker_Create_fail = 0;
	currentBroker_module_count = 0;
	currentBroker_ref_count = 0;

	currentModuleLoader_Load_call = 0;
	whenShallModuleLoader_Load_fail = 0;

	currentModule_Create_call = 0;
	currentModule_Destroy_call = 0;

	currentVECTOR_create_call = 0;
	whenShallVECTOR_create_fail = 0;
	currentVECTOR_push_back_call = 0;
	whenShallVECTOR_push_back_fail = 0;
	currentVECTOR_find_if_call = 0;
	whenShallVECTOR_find_if_fail = 0;

	dummyAPIs = {
		mock_Module_Create,
		mock_Module_Destroy,
		mock_Module_Receive
	};

	GATEWAY_MODULES_ENTRY dummyEntry = {
		"dummy module",
		DUMMY_LIBRARY_PATH,
		NULL
	};

	dummyProps = (GATEWAY_PROPERTIES*)malloc(sizeof(GATEWAY_PROPERTIES));
	dummyProps->gateway_modules = BASEIMPLEMENTATION::VECTOR_create(sizeof(GATEWAY_MODULES_ENTRY));
	dummyProps->gateway_links = BASEIMPLEMENTATION::VECTOR_create(sizeof(GATEWAY_LINK_ENTRY));
	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_modules, &dummyEntry, 1);
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
	if (!MicroMockReleaseMutex(g_testByTest))
	{
		ASSERT_FAIL("failure in test framework at ReleaseMutex");
	}
	currentmalloc_call = 0;
	whenShallmalloc_fail = 0;

	currentBroker_Create_call = 0;
	whenShallBroker_Create_fail = 0;

	BASEIMPLEMENTATION::VECTOR_destroy(dummyProps->gateway_modules);
	free(dummyProps);
}

/*Tests_SRS_GATEWAY_LL_14_001: [This function shall create a GATEWAY_HANDLE representing the newly created gateway.]*/
/*Tests_SRS_GATEWAY_LL_14_003: [This function shall create a new BROKER_HANDLE for the gateway representing this gateway's message broker. ]*/
/*Tests_SRS_GATEWAY_LL_14_033: [ The function shall create a vector to store each MODULE_DATA. ]*/
/*Tests_SRS_GATEWAY_LL_04_001: [ The function shall create a vector to store each LINK_DATA ] */
TEST_FUNCTION(Gateway_LL_Create_Creates_Handle_Success)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Expectations
	STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_Create());
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1);
	expectEventSystemInit(mocks);

	//Act
	GATEWAY_HANDLE gateway = Gateway_LL_Create(NULL);

	//Assert
	ASSERT_IS_NOT_NULL(gateway);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gateway);
}

/*Tests_SRS_GATEWAY_LL_14_002: [This function shall return NULL upon any memory allocation failure.]*/
TEST_FUNCTION(Gateway_LL_Create_Creates_Handle_Malloc_Failure)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Expectations
	whenShallmalloc_fail = 1;
	STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
		.IgnoreArgument(1);

	//Act
	GATEWAY_HANDLE gateway = Gateway_LL_Create(NULL);

	//Assert
	ASSERT_IS_NULL(gateway);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	//Nothing to cleanup
}

/*Tests_SRS_GATEWAY_LL_14_004: [ This function shall return NULL if a BROKER_HANDLE cannot be created. ]*/
TEST_FUNCTION(Gateway_LL_Create_Cannot_Create_Broker_Handle_Failure)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Expectations
	STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
		.IgnoreArgument(1);
	whenShallBroker_Create_fail = 1;
	STRICT_EXPECTED_CALL(mocks, Broker_Create());
	STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
		.IgnoreArgument(1);

	GATEWAY_HANDLE gateway = Gateway_LL_Create(NULL);

	//Assert
	ASSERT_IS_NULL(gateway);
	mocks.AssertActualAndExpectedCalls();
	
	//Cleanup
	//Nothing to cleanup
}

/*Tests_SRS_GATEWAY_LL_14_034: [ This function shall return NULL if a VECTOR_HANDLE cannot be created. ]*/
/*Tests_SRS_GATEWAY_LL_14_035: [ This function shall destroy the previously created BROKER_HANDLE and free the GATEWAY_HANDLE if the VECTOR_HANDLE cannot be created. ]*/
TEST_FUNCTION(Gateway_LL_Create_VECTOR_Create_Fails)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Expectations
	STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
		.IgnoreArgument(1);

	STRICT_EXPECTED_CALL(mocks, Broker_Create());

	whenShallVECTOR_create_fail = 1; 
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1);

	STRICT_EXPECTED_CALL(mocks, Broker_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
		.IgnoreArgument(1);


	//Act
	GATEWAY_HANDLE gateway = Gateway_LL_Create(NULL);
	
	//Assert
	ASSERT_IS_NULL(gateway);

	//Cleanup
	//Nothing to cleanup
}

TEST_FUNCTION(Gateway_LL_Create_VECTOR_push_back_Fails_To_Add_All_Modules_In_Props)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Add another entry to the properties
	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module 2",
		"x2.dll",
		NULL
	};

	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_modules, &dummyEntry2, 1);

	//Expectations
	STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_Create());
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1); //modules
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1); //links
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(dummyProps->gateway_modules));

	//Adding module 1 (Success)
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(dummyProps->gateway_modules, 0));
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, NULL))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_IncRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	//Adding module 2 (Failure)
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(dummyProps->gateway_modules, 1));
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, NULL))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_IncRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	whenShallVECTOR_push_back_fail = 2;
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_RemoveModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_DecRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);

	//Removing previous module in Gateway_LL_Destroy()
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_front(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_RemoveModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_DecRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_erase(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
		.IgnoreArgument(1);

	GATEWAY_HANDLE gateway = Gateway_LL_Create(dummyProps);

	ASSERT_IS_NULL(gateway);
	ASSERT_ARE_EQUAL(size_t, 0, currentBroker_module_count);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gateway);

}

/*Tests_SRS_GATEWAY_LL_14_036: [ If any MODULE_HANDLE is unable to be created from a GATEWAY_MODULES_ENTRY the GATEWAY_HANDLE will be destroyed. ]*/
TEST_FUNCTION(Gateway_LL_Create_Broker_AddModule_Fails_To_Add_All_Modules_In_Props)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Add another entry to the properties
	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module 2",
		"x2.dll",
		NULL
	};

	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_modules, &dummyEntry2, 1);

	//Expectations
	STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_Create());
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1); //modules
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1); //links
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(dummyProps->gateway_modules));

	//Adding module 1 (Success)
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(dummyProps->gateway_modules, 0));
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, NULL))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_IncRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	//Adding module 2 (Failure)
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(dummyProps->gateway_modules, 1));
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, NULL))
		.IgnoreArgument(1);
	whenShallBroker_AddModule_fail = 2;
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);

	//Removing previous module in Gateway_LL_Destroy()
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_front(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_RemoveModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_DecRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_erase(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1); //Modules
	STRICT_EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1); //Links
	STRICT_EXPECTED_CALL(mocks, Broker_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
		.IgnoreArgument(1);

	GATEWAY_HANDLE gateway = Gateway_LL_Create(dummyProps);

	ASSERT_IS_NULL(gateway);
	ASSERT_ARE_EQUAL(size_t, 0, currentBroker_module_count);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gateway);

}


/*Tests_SRS_GATEWAY_LL_04_004: [If a module with the same module_name already exists, this function shall fail and the GATEWAY_HANDLE will be destroyed.]*/
TEST_FUNCTION(Gateway_LL_Create_AddModule_WithDuplicatedModuleName_Fails)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Add another entry to the properties
	GATEWAY_MODULES_ENTRY duplicatedEntry = {
		"dummy module",
		DUMMY_LIBRARY_PATH,
		NULL
	};

	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_modules, &duplicatedEntry, 1);
	
	//Expectations
	STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_Create());
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1); //modules
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1); //links
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(dummyProps->gateway_modules));

	//Adding module 1 (Success)
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(dummyProps->gateway_modules, 0));
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, NULL))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_IncRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	//Adding module 2 (Failure)
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(dummyProps->gateway_modules, 1));
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();

	//Removing previous module
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_front(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_RemoveModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_DecRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_erase(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
		.IgnoreArgument(1);

	GATEWAY_HANDLE gateway = Gateway_LL_Create(dummyProps);

	ASSERT_IS_NULL(gateway);
	ASSERT_ARE_EQUAL(size_t, 0, currentBroker_module_count);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gateway);

}


/*Tests_SRS_GATEWAY_LL_14_009: [ The function shall use each of GATEWAY_PROPERTIES's gateway_modules to create and add a module to the gateway's message broker. ]*/
TEST_FUNCTION(Gateway_LL_Create_Adds_All_Modules_In_Props_Success)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Add another entry to the properties
	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module 2",
		"x2.dll",
		NULL
	};

	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_modules, &dummyEntry2, 1);

	//Expectations
	STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_Create());
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1); //modules vector.
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1); //links vector.
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(dummyProps->gateway_modules)); //Modules

	//Adding module 1 (Success)
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(dummyProps->gateway_modules, 0));
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, NULL))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_IncRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	//Adding module 2 (Success)
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(dummyProps->gateway_modules, 1));
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, NULL))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_IncRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	STRICT_EXPECTED_CALL(mocks, VECTOR_size(dummyProps->gateway_links)); //Links

	expectEventSystemInit(mocks);

	//Act
	GATEWAY_HANDLE gateway = Gateway_LL_Create(dummyProps);

	//Assert
	ASSERT_IS_NOT_NULL(gateway);
	ASSERT_ARE_EQUAL(size_t, 2, currentBroker_module_count);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gateway);
}


/*Tests_SRS_GATEWAY_LL_04_002: [ The function shall use each GATEWAY_LINK_ENTRY of GATEWAY_PROPERTIES's gateway_links to add a LINK to GATEWAY_HANDLE's broker. ] */
TEST_FUNCTION(Gateway_LL_Create_Adds_All_Modules_And_All_Links_In_Props_Success)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Add another entry to the properties
	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module 2",
		"x2.dll",
		NULL
	};

	GATEWAY_LINK_ENTRY dummyLink = {
		"dummy module",
		"dummy module 2"
	};

	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_modules, &dummyEntry2, 1);
	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_links, &dummyLink, 1);

	//Expectations
	STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_Create());
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1); //modules vector.
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1); //links vector.
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(dummyProps->gateway_modules)); //Modules

																		   //Adding module 1 (Success)
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(dummyProps->gateway_modules, 0));
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, NULL))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_IncRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	//Adding module 2 (Success)
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(dummyProps->gateway_modules, 1));
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, NULL))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_IncRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	STRICT_EXPECTED_CALL(mocks, VECTOR_size(dummyProps->gateway_links)); //Links


	//Adding link1 (Success)
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(dummyProps->gateway_links, 0));
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments(); //Check if Link exists.
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments(); //Check if Source Module exists.
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments(); //Check if Sink Module exists.
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	expectEventSystemInit(mocks);

	//Act
	GATEWAY_HANDLE gateway = Gateway_LL_Create(dummyProps);

	//Assert
	ASSERT_IS_NOT_NULL(gateway);
	ASSERT_ARE_EQUAL(size_t, 2, currentBroker_module_count);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gateway);
}

/*Tests_SRS_GATEWAY_LL_04_003: [If any GATEWAY_LINK_ENTRY is unable to be added to the broker the GATEWAY_HANDLE will be destroyed.]*/
TEST_FUNCTION(Gateway_LL_Create_Adds_All_Modules_And_Links_fromNonExistingModule_Fail)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Add another entry to the properties
	GATEWAY_LINK_ENTRY dummyLink = {
		"Non Existing Module",
		"dummy module 2"
	};
	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_links, &dummyLink, 1);

	//Expectations
	STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_Create());
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1); //modules vector.
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.IgnoreArgument(1); //links vector.
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(dummyProps->gateway_modules)); //Modules

	//Adding module 1 (Success)
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(dummyProps->gateway_modules, 0));
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, NULL))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_IncRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	STRICT_EXPECTED_CALL(mocks, VECTOR_size(dummyProps->gateway_links)); //Links

	//Adding link1 (Failure)
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(dummyProps->gateway_links, 0));
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments(); //Check if Link exists.
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments(); //Check if Source Module exists.


	//Removing previous module
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_front(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_RemoveModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_DecRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_erase(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1); //Modules
	STRICT_EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1); //Links
	STRICT_EXPECTED_CALL(mocks, Broker_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
		.IgnoreArgument(1);

	GATEWAY_HANDLE gateway = Gateway_LL_Create(dummyProps);

	ASSERT_IS_NULL(gateway);
	ASSERT_ARE_EQUAL(size_t, 0, currentBroker_module_count);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gateway);
}

/*Tests_SRS_GATEWAY_LL_14_005: [ If gw is NULL the function shall do nothing. ]*/
TEST_FUNCTION(Gateway_LL_Destroy_Does_Nothing_If_NULL)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gateway = NULL;

	//Act
	Gateway_LL_Destroy(gateway);

	//Assert
	mocks.AssertActualAndExpectedCalls();
}

/*Tests_SRS_GATEWAY_LL_14_037: [ If GATEWAY_HANDLE_DATA's message broker cannot remove a module, the function shall log the error and continue removing modules from the GATEWAY_HANDLE. ]*/
TEST_FUNCTION(Gateway_LL_Destroy_Continues_Unloading_If_Broker_RemoveModule_Fails)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Add another entry to the properties
	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module 2",
		"x2.dll",
		NULL
	};

	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_modules, &dummyEntry2, 1);

	GATEWAY_HANDLE gateway = Gateway_LL_Create(dummyProps);
	mocks.ResetAllCalls();

	//Expectations
	expectEventSystemDestroy(mocks);

	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1); //Modules
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1); //Links
	STRICT_EXPECTED_CALL(mocks, VECTOR_front(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_RemoveModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_DecRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_erase(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_front(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_RemoveModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_DecRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_erase(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1); //Modules
	STRICT_EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1); //links
	STRICT_EXPECTED_CALL(mocks, Broker_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
		.IgnoreArgument(1);

	//Act
	Gateway_LL_Destroy(gateway);

	//Assert
	mocks.AssertActualAndExpectedCalls();
}

/*Tests_SRS_GATEWAY_LL_14_028: [ The function shall remove each module in GATEWAY_HANDLE_DATA's modules vector and destroy GATEWAY_HANDLE_DATA's modules. ]*/
/*Tests_SRS_GATEWAY_LL_14_006: [ The function shall destroy the GATEWAY_HANDLE_DATA's broker BROKER_HANDLE. ]*/
/*Tests_SRS_GATEWAY_LL_04_014: [ The function shall remove each link in GATEWAY_HANDLE_DATA's links vector and destroy GATEWAY_HANDLE_DATA's link. ]*/
TEST_FUNCTION(Gateway_LL_Destroy_Removes_All_Modules_And_Destroys_Vector_Success)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Add another entry to the properties
	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module 2",
		"x2.dll",
		NULL
	};
	
	GATEWAY_LINK_ENTRY dummyLink = {
		"dummy module",
		"dummy module 2"
	};

	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_modules, &dummyEntry2, 1);
	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_links, &dummyLink, 1);

	GATEWAY_HANDLE gateway = Gateway_LL_Create(dummyProps);
	mocks.ResetAllCalls();

	//Gateway_LL_Destroy Expectations
	expectEventSystemDestroy(mocks);

	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_front(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_RemoveModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_DecRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_erase(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_front(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_RemoveModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_DecRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_erase(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);


	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_front(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_erase(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1); //Modules
	STRICT_EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG))
		.IgnoreArgument(1); //Links
	STRICT_EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1); //Modules.
	STRICT_EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1); //Links
	STRICT_EXPECTED_CALL(mocks, Broker_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
		.IgnoreArgument(1);

	//Act
	Gateway_LL_Destroy(gateway);

	//Assert
	ASSERT_ARE_EQUAL(size_t, 0, currentBroker_module_count);
	mocks.AssertActualAndExpectedCalls();
}

/*Tests_SRS_GATEWAY_LL_14_011: [ If gw, entry, or GATEWAY_MODULES_ENTRY's module_path is NULL the function shall return NULL. ]*/
TEST_FUNCTION(Gateway_LL_AddModule_Returns_Null_For_Null_Gateway)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetAllCalls();

	//Act
	MODULE_HANDLE handle0 = Gateway_LL_AddModule(NULL, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));
	
	//Assert
	ASSERT_IS_NULL(handle0);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_14_011: [ If gw, entry, or GATEWAY_MODULES_ENTRY's module_path is NULL the function shall return NULL. ]*/
TEST_FUNCTION(Gateway_LL_AddModule_Returns_Null_For_Null_Module)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetAllCalls();

	//Act
	MODULE_HANDLE handle1 = Gateway_LL_AddModule(gw, NULL);

	//Assert
	ASSERT_IS_NULL(handle1);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_14_011: [ If gw, entry, or GATEWAY_MODULES_ENTRY's module_path is NULL the function shall return NULL. ]*/
TEST_FUNCTION(Gateway_LL_AddModule_Returns_Null_For_Null_Params)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetAllCalls();

	//Act
	GATEWAY_MODULES_ENTRY* entry = (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules);
	entry->module_path = NULL;
	MODULE_HANDLE handle2 = Gateway_LL_AddModule(gw, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));

	//Assert
	ASSERT_IS_NULL(handle2);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_14_012: [ The function shall load the module located at GATEWAY_MODULES_ENTRY's module_path into a MODULE_LIBRARY_HANDLE. ]*/
/*Tests_SRS_GATEWAY_LL_14_013: [ The function shall get the const MODULE_APIS* from the MODULE_LIBRARY_HANDLE. ]*/
/*Tests_SRS_GATEWAY_LL_14_017: [ The function shall attach the module to the GATEWAY_HANDLE_DATA's broker using a call to Broker_AddModule. ]*/
/*Tests_SRS_GATEWAY_LL_14_029: [ The function shall create a new MODULE_DATA containting the MODULE_HANDLE and MODULE_LIBRARY_HANDLE if the module was successfully linked to the message broker. ]*/
/*Tests_SRS_GATEWAY_LL_14_032: [ The function shall add the new MODULE_DATA to GATEWAY_HANDLE_DATA's modules if the module was successfully linked to the message broker. ]*/
/*Tests_SRS_GATEWAY_LL_14_019: [ The function shall return the newly created MODULE_HANDLE only if each API call returns successfully. ]*/
/*Tests_SRS_GATEWAY_LL_14_039: [ The function shall increment the BROKER_HANDLE reference count if the MODULE_HANDLE was successfully linked to the GATEWAY_HANDLE_DATA's broker. ]*/
/*Tests_SRS_GATEWAY_LL_99_011: [ The function shall assign `module_apis` to `MODULE::module_apis`. ]*/
TEST_FUNCTION(Gateway_LL_AddModule_Loads_Module_From_Library_Path)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetAllCalls();

	//Expectations
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(DUMMY_LIBRARY_PATH));
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_IncRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	//Act
	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));

	//Assert
	ASSERT_IS_NOT_NULL(handle);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_14_031: [ If unsuccessful, the function shall return NULL. ]*/
TEST_FUNCTION(Gateway_LL_AddModule_Fails)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetActualCalls();

	//Expectations
	whenShallModuleLoader_Load_fail = 1;
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(IGNORED_PTR_ARG))
		.IgnoreArgument(1);

	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));

	//Assert
	ASSERT_IS_NULL(handle);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_14_015: [ The function shall use the MODULE_APIS to create a MODULE_HANDLE using the GATEWAY_MODULES_ENTRY's module_properties. ]*/
/*Tests_SRS_GATEWAY_LL_14_039: [ The function shall increment the BROKER_HANDLE reference count if the MODULE_HANDLE was successfully added to the GATEWAY_HANDLE_DATA's broker. ]*/
TEST_FUNCTION(Gateway_LL_AddModule_Creates_Module_Using_Module_Properties)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetAllCalls();
	bool* properties = (bool*)malloc(sizeof(bool));
	*properties = true;
	GATEWAY_MODULES_ENTRY entry = {
		"Test module",
		DUMMY_LIBRARY_PATH,
		properties
	};

	//Expectations
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, properties))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_IncRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	//Act
	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, &entry);

	//Assert
	ASSERT_IS_NOT_NULL(handle);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
	free(properties);
}

/*Tests_SRS_GATEWAY_LL_14_016: [ If the module creation is unsuccessful, the function shall return NULL. ]*/
TEST_FUNCTION(Gateway_LL_AddModule_Module_Create_Fails)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetAllCalls();
	//Setting this boolean to false will cause mock_Module_Create to fail
	bool properties = false;
	GATEWAY_MODULES_ENTRY entry = {
		"Test module",
		DUMMY_LIBRARY_PATH,
		&properties
	};

	//Expectations
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(DUMMY_LIBRARY_PATH));
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);

	//Act
	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, &entry);

	//Assert
	ASSERT_IS_NULL(handle);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_14_018: [ If the function cannot attach the module to the message broker, the function shall return NULL. ]*/
TEST_FUNCTION(Gateway_LL_AddModule_Broker_AddModule_Fails)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetAllCalls();
	
	//Expectations
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	whenShallBroker_AddModule_fail = 1;
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	

	//Act
	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));

	ASSERT_IS_NULL(handle);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_14_030: [ If any internal API call is unsuccessful after a module is created, the library will be unloaded and the module destroyed. ]*/
/*Tests_SRS_GATEWAY_LL_14_039: [The function shall increment the BROKER_HANDLE reference count if the MODULE_HANDLE was successfully linked to the GATEWAY_HANDLE_DATA's broker. ]*/
TEST_FUNCTION(Gateway_LL_AddModule_Internal_API_Fail_Rollback_Module)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetAllCalls();

	//Expectations
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Load(DUMMY_LIBRARY_PATH));
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Create(IGNORED_PTR_ARG, NULL))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, Broker_AddModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_IncRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	whenShallVECTOR_push_back_fail = 1;
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_RemoveModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_DecRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);

	//Act
	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));

	ASSERT_IS_NULL(handle);
	ASSERT_ARE_EQUAL(size_t, 0, currentBroker_module_count);
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_14_020: [ If gw or module is NULL the function shall return. ]*/
TEST_FUNCTION(Gateway_LL_RemoveModule_Does_Nothing_If_Gateway_NULL)
{
	//Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));
	mocks.ResetAllCalls();

	//Act
	Gateway_LL_RemoveModule(NULL, NULL);
	Gateway_LL_RemoveModule(NULL, handle);
	
	//Assert
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_14_020: [ If gw or module is NULL the function shall return. ]*/
TEST_FUNCTION(Gateway_LL_RemoveModule_Does_Nothing_If_Module_NULL)
{
	//Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetAllCalls();

	//Expectations
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();

	//Act
	Gateway_LL_RemoveModule(gw, NULL);

	//Assert
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_14_023: [ The function shall locate the MODULE_DATA object in GATEWAY_HANDLE_DATA's modules containing module and return if it cannot be found. ]*/
/*Tests_SRS_GATEWAY_LL_14_021: [ The function shall detach module from the GATEWAY_HANDLE_DATA's broker BROKER_HANDLE. ]*/
/*Tests_SRS_GATEWAY_LL_14_024: [ The function shall use the MODULE_DATA's module_library_handle to retrieve the MODULE_APIS and destroy module. ]*/
/*Tests_SRS_GATEWAY_LL_14_025: [ The function shall unload MODULE_DATA's module_library_handle. ]*/
/*Tests_SRS_GATEWAY_LL_14_026: [ The function shall remove that MODULE_DATA from GATEWAY_HANDLE_DATA's modules. ]*/
/*Tests_SRS_GATEWAY_LL_14_038: [ The function shall decrement the BROKER_HANDLE reference count. ]*/
TEST_FUNCTION(Gateway_LL_RemoveModule_Finds_Module_Data_Success)
{
	//Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));
	mocks.ResetAllCalls();

	//Expectations
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, Broker_RemoveModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, Broker_DecRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(handle))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_erase(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	

	//Act
	Gateway_LL_RemoveModule(gw, handle);

	//Assert
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_14_023: [ The function shall locate the MODULE_DATA object in GATEWAY_HANDLE_DATA's modules containing module and return if it cannot be found. ]*/
TEST_FUNCTION(Gateway_LL_RemoveModule_Finds_Module_Data_Failure)
{
	//Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));
	mocks.ResetAllCalls();

	//Expectations
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, gw))
		.IgnoreAllArguments();

	//Act
	Gateway_LL_RemoveModule(gw, (MODULE_HANDLE)gw);

	//Assert
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_14_022: [ If GATEWAY_HANDLE_DATA's broker cannot detach module, the function shall log the error and continue unloading the module from the GATEWAY_HANDLE. ]*/
/*Tests_SRS_GATEWAY_LL_14_038: [ The function shall decrement the BROKER_HANDLE reference count. ]*/
TEST_FUNCTION(Gateway_LL_RemoveModule_Broker_RemoveModule_Failure)
{
	//Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));

	mocks.ResetAllCalls();

	//Expectations
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	whenShallBroker_RemoveModule_fail = 1;
	STRICT_EXPECTED_CALL(mocks, Broker_RemoveModule(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreArgument(1)
		.IgnoreArgument(2);
	STRICT_EXPECTED_CALL(mocks, Broker_DecRef(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_GetModuleAPIs(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, mock_Module_Destroy(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, ModuleLoader_Unload(IGNORED_PTR_ARG))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_erase(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	//Act
	Gateway_LL_RemoveModule(gw, handle);

	//Assert
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_04_005: [ If gw or entryLink is NULL the function shall return. ]*/
TEST_FUNCTION(Gateway_LL_RemoveLink_Does_Nothing_If_Gateway_NULL)
{
	//Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));

	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module2",
		DUMMY_LIBRARY_PATH,
		NULL
	};

	GATEWAY_LINK_ENTRY dummyLink = {
		"dummy module",
		"dummy module2"
	};

	MODULE_HANDLE handle2 = Gateway_LL_AddModule(gw, &dummyEntry2);

	Gateway_LL_AddLink(gw, (GATEWAY_LINK_ENTRY*)&dummyLink);

	mocks.ResetAllCalls();

	//Act
	Gateway_LL_RemoveLink(NULL, NULL);
	Gateway_LL_RemoveLink(NULL, &dummyLink);

	//Assert
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_04_005: [ If gw or entryLink is NULL the function shall return. ]*/
TEST_FUNCTION(Gateway_LL_RemoveLink_Does_Nothing_If_entryLink_NULL)
{
	//Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetAllCalls();

	//Act
	Gateway_LL_RemoveLink(gw, NULL);

	//Assert
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_04_006: [ The function shall locate the LINK_DATA object in GATEWAY_HANDLE_DATA's links containing link and return if it cannot be found. ]*/
TEST_FUNCTION(Gateway_LL_RemoveLink_NonExistingSourceModule_Find_Link_Data_Failure)
{
	//Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));

	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module2",
		DUMMY_LIBRARY_PATH,
		NULL
	};

	GATEWAY_LINK_ENTRY dummyLink = {
		"dummy module",
		"dummy module2"
	};

	GATEWAY_LINK_ENTRY dummyLink2 = {
		"NonExistingLink",
		"dummy module"		
	};

	MODULE_HANDLE handle2 = Gateway_LL_AddModule(gw, &dummyEntry2);

	Gateway_LL_AddLink(gw, (GATEWAY_LINK_ENTRY*)&dummyLink);

	mocks.ResetAllCalls();


	//Expectations
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, gw))
		.IgnoreAllArguments();

	//Act
	Gateway_LL_RemoveLink(gw, &dummyLink2);

	//Assert
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_04_006: [ The function shall locate the LINK_DATA object in GATEWAY_HANDLE_DATA's links containing link and return if it cannot be found. ]*/
TEST_FUNCTION(Gateway_LL_RemoveLink_NonExistingSinkModule_Find_Link_Data_Failure)
{
	//Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));

	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module2",
		DUMMY_LIBRARY_PATH,
		NULL
	};

	GATEWAY_LINK_ENTRY dummyLink = {
		"dummy module",
		"dummy module2"
	};

	GATEWAY_LINK_ENTRY dummyLink2 = {
		"dummy module",
		"NonExistingLink"
	};

	MODULE_HANDLE handle2 = Gateway_LL_AddModule(gw, &dummyEntry2);

	Gateway_LL_AddLink(gw, (GATEWAY_LINK_ENTRY*)&dummyLink);

	mocks.ResetAllCalls();


	//Expectations
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();

	//Act
	Gateway_LL_RemoveLink(gw, &dummyLink2);

	//Assert
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_04_006: [ The function shall locate the LINK_DATA object in GATEWAY_HANDLE_DATA's links containing link and return if it cannot be found. ]*/
/*Tests_SRS_GATEWAY_LL_04_007: [ The functional shall remove that LINK_DATA from GATEWAY_HANDLE_DATA's links. ]*/
TEST_FUNCTION(Gateway_LL_RemoveLink_Finds_Link_Data_Success)
{
	//Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	MODULE_HANDLE handle = Gateway_LL_AddModule(gw, (GATEWAY_MODULES_ENTRY*)BASEIMPLEMENTATION::VECTOR_front(dummyProps->gateway_modules));

	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module2",
		DUMMY_LIBRARY_PATH,
		NULL
	};

	GATEWAY_LINK_ENTRY dummyLink = {
		"dummy module",
		"dummy module2"
	};

	GATEWAY_LINK_ENTRY dummyLink2 = {
		"dummy module",
		"dummy module2"
	};

	MODULE_HANDLE handle2 = Gateway_LL_AddModule(gw, &dummyEntry2);

	Gateway_LL_AddLink(gw, (GATEWAY_LINK_ENTRY*)&dummyLink);

	mocks.ResetAllCalls();

	//Expectations
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();
	STRICT_EXPECTED_CALL(mocks, VECTOR_erase(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);


	//Act
	Gateway_LL_RemoveLink(gw, &dummyLink2);

	//Assert
	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_04_008: [ If gw , entryLink, entryLink->module_source or entryLink->module_source is NULL the function shall return GATEWAY_ADD_LINK_INVALID_ARG. ]*/
TEST_FUNCTION(Gateway_LL_AddLink_with_Null_Gateway_Fail)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	GATEWAY_LINK_ENTRY dummyLink2 = {
		"dummy module",
		"dummy module2"
	};

	mocks.ResetAllCalls();
	
	//Act
	GATEWAY_ADD_LINK_RESULT result = Gateway_LL_AddLink(NULL, &dummyLink2);

	//Assert
	ASSERT_ARE_EQUAL(GATEWAY_ADD_LINK_RESULT, GATEWAY_ADD_LINK_INVALID_ARG, result);

	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_04_008: [ If gw , entryLink, entryLink->module_source or entryLink->module_source is NULL the function shall return GATEWAY_ADD_LINK_INVALID_ARG. ]*/
TEST_FUNCTION(Gateway_LL_AddLink_with_Null_Link_Fail)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	GATEWAY_LINK_ENTRY dummyLink2 = {
		"dummy module",
		"dummy module2"
	};

	mocks.ResetAllCalls();

	//Act
	GATEWAY_ADD_LINK_RESULT result = Gateway_LL_AddLink(gw, NULL);

	//Assert
	ASSERT_ARE_EQUAL(GATEWAY_ADD_LINK_RESULT, GATEWAY_ADD_LINK_INVALID_ARG, result);

	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_04_008: [ If gw , entryLink, entryLink->module_source or entryLink->module_source is NULL the function shall return GATEWAY_ADD_LINK_INVALID_ARG. ]*/
TEST_FUNCTION(Gateway_LL_AddLink_with_Null_Link_Module_Source_Fail)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	GATEWAY_LINK_ENTRY dummyLink2;
	dummyLink2.module_source = NULL;
	dummyLink2.module_sink = "Test";

	mocks.ResetAllCalls();

	//Act
	GATEWAY_ADD_LINK_RESULT result = Gateway_LL_AddLink(gw, &dummyLink2);

	//Assert
	ASSERT_ARE_EQUAL(GATEWAY_ADD_LINK_RESULT, GATEWAY_ADD_LINK_INVALID_ARG, result);

	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/* Tests_SRS_GATEWAY_LL_26_001: [ This function shall initialize attached Event System and report GATEWAY_CREATED event. ]*/
/* Tests_SRS_GATEWAY_LL_26_010: [ This function shall report `GATEWAY_MODULE_LIST_CHANGED` event. ] */
TEST_FUNCTION(Gateway_LL_Event_System_Create_And_Report)
{
	//Arrange
	CGatewayLLMocks mocks;
	
	//Expectations
	EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
	EXPECTED_CALL(mocks, Broker_Create());
	EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG)); //Modules.
	EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG)); //Links
	
	expectEventSystemInit(mocks);

	//Act
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);

	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}

/* Tests_SRS_GATEWAY_LL_26_002: [ If Event System module fails to be initialized the gateway module shall be destroyed and NULL returned with no events reported. ] */
TEST_FUNCTION(Gateway_LL_Event_System_Create_Fail)
{
	// Arrange
	CGatewayLLMocks mocks;

	//Expectations
	EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
	EXPECTED_CALL(mocks, Broker_Create());
	EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG)); //Modules.
	EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG)); //Links
	// Fail to create
	EXPECTED_CALL(mocks, EventSystem_Init())
		.SetFailReturn((EVENTSYSTEM_HANDLE)NULL);
	// Note - no EventSystem_Report()!
	// Gateway_destroy called from inside create
	EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG)); //For Modules.
	EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG)); //For Links.
	EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG)); //Modules
	EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG)); //Links
	EXPECTED_CALL(mocks, Broker_Destroy(IGNORED_PTR_ARG));
	EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

	//Act
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);

	//Assert
	ASSERT_IS_NULL(gw);
	mocks.AssertActualAndExpectedCalls();
}

/* Tests_SRS_GATEWAY_LL_26_003: [ If the Event System module is initialized, this function shall report GATEWAY_DESTROYED event. ] */
/* Tests_SRS_GATEWAY_LL_26_004: [ This function shall destroy the attached Event System. ] */
TEST_FUNCTION(Gateway_LL_Event_System_Report_Destroy)
{
	// Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetAllCalls();

	// Expectations
	expectEventSystemDestroy(mocks);
	EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG)); //For Modules.
	EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG)); //For Links
	EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG)); //Modules
	EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG)); //Links
	EXPECTED_CALL(mocks, Broker_Destroy(IGNORED_PTR_ARG));
	EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

	// Act
	Gateway_LL_Destroy(gw);

	// Assert
	mocks.AssertActualAndExpectedCalls();
}

/* Tests_SRS_GATEWAY_LL_26_006: [** This function shall log a failure and do nothing else when `gw` parameter is NULL. ] */
TEST_FUNCTION(Gateway_LL_AddEventCallback_NULL_Gateway)
{
	// Arrange
	CGatewayLLMocks mocks;

	// Expectations
	// Empty! No callbacks should happen at all

	// Act
	Gateway_AddEventCallback(NULL, GATEWAY_CREATED, NULL);
	Gateway_AddEventCallback(NULL, GATEWAY_DESTROYED, NULL);
	Gateway_AddEventCallback(NULL, GATEWAY_CREATED, sampleCallbackFunc);

	// Assert
	ASSERT_ARE_EQUAL(int, sampleCallbackFuncCallCount, 0);
	mocks.AssertActualAndExpectedCalls();
}

/*Tests_SRS_GATEWAY_LL_26_007: [ This function shall return a snapshot copy of information about current gateway modules. ]*/
TEST_FUNCTION(Gateway_LL_GetModuleList_Basic)
{
	// Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(dummyProps);
	mocks.ResetAllCalls();

	// Expect
	EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG));
	EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG));
	EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
	EXPECTED_CALL(mocks, VECTOR_element(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
	EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
	EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

	// Act
	VECTOR_HANDLE modules = Gateway_GetModuleList(gw);

	// Assert
	ASSERT_IS_NOT_NULL(modules);
	ASSERT_ARE_EQUAL(int, BASEIMPLEMENTATION::VECTOR_size(modules), 1); 
	ASSERT_ARE_EQUAL(
		int,
		0,
		strcmp(
			((GATEWAY_MODULE_INFO*)BASEIMPLEMENTATION::VECTOR_element(modules, 0))->module_name,
			"dummy module")
	);
	mocks.AssertActualAndExpectedCalls();

	// Cleanup
	VECTOR_destroy(modules);
	Gateway_LL_Destroy(gw);
	mocks.ResetAllCalls();
}

/*Tests_SRS_GATEWAY_LL_26_008: [ If the `gw` parameter is NULL, the function shall return NULL handle and not allocate any data. ] */
TEST_FUNCTION(Gateway_LL_GetModuleList_NULL_Gateway)
{
	// Arrange
	CGatewayLLMocks mocks;

	// Expectations
	// Empty

	// Act
	VECTOR_HANDLE vector = Gateway_GetModuleList(NULL);

	// Assert
	ASSERT_IS_NULL(vector);
	mocks.AssertActualAndExpectedCalls();
}

/*Tests_SRS_GATEWAY_LL_26_009: [ This function shall return a NULL handle should any internal callbacks fail. ]*/
TEST_FUNCTION(Gateway_LL_GetModuleList_Vector_Create_Fail)
{
	// Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetAllCalls();

	// Expectations
	EXPECTED_CALL(mocks, VECTOR_create(IGNORED_NUM_ARG))
		.SetFailReturn((VECTOR_HANDLE)NULL);

	// Act
	VECTOR_HANDLE vector = Gateway_GetModuleList(gw);

	// Assert
	ASSERT_IS_NULL(vector);
	mocks.AssertActualAndExpectedCalls();

	// Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_26_009: [ This function shall return a NULL handle should any internal callbacks fail. ]*/
TEST_FUNCTION(Gateway_LL_GetModuleList_push_back_fail)
{
	// Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(dummyProps);
	mocks.ResetAllCalls();

	// Expectations
	STRICT_EXPECTED_CALL(mocks, VECTOR_create(sizeof(GATEWAY_MODULE_INFO)));
	EXPECTED_CALL(mocks, VECTOR_size(IGNORED_PTR_ARG));
	EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG));
	STRICT_EXPECTED_CALL(mocks, VECTOR_element(IGNORED_PTR_ARG, 0))
		.IgnoreArgument(1);
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2)
		.SetFailReturn(1);
	// destroy after fail
	EXPECTED_CALL(mocks, VECTOR_destroy(IGNORED_PTR_ARG));
	EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG));

	// Act
	VECTOR_HANDLE vector = Gateway_GetModuleList(gw);

	// Assert
	ASSERT_IS_NULL(vector);
	mocks.AssertActualAndExpectedCalls();

	// Cleanup
	Gateway_LL_Destroy(gw);
}

TEST_FUNCTION(Gateway_LL_AddEventCallback_Forwards)
{
	// Arrange
	CGatewayLLMocks mocks;
	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	mocks.ResetAllCalls();

	// Expectations
	STRICT_EXPECTED_CALL(mocks, EventSystem_AddEventCallback(IGNORED_PTR_ARG, GATEWAY_MODULE_LIST_CHANGED, sampleCallbackFunc))
		.IgnoreArgument(1);

	// Act
	Gateway_AddEventCallback(gw, GATEWAY_MODULE_LIST_CHANGED, sampleCallbackFunc);

	// Assert
	mocks.AssertActualAndExpectedCalls();

	// Cleanup
	Gateway_LL_Destroy(gw);
}

/*Tests_SRS_GATEWAY_LL_04_008: [ If gw , entryLink, entryLink->module_source or entryLink->module_source is NULL the function shall return GATEWAY_ADD_LINK_INVALID_ARG. ]*/
TEST_FUNCTION(Gateway_LL_AddLink_with_Null_Link_Module_Sink_Fail)
{
	//Arrange
	CGatewayLLMocks mocks;

	GATEWAY_HANDLE gw = Gateway_LL_Create(NULL);
	GATEWAY_LINK_ENTRY dummyLink2;
	dummyLink2.module_source = "Test";
	dummyLink2.module_sink = NULL;

	mocks.ResetAllCalls();

	//Act
	GATEWAY_ADD_LINK_RESULT result = Gateway_LL_AddLink(gw, &dummyLink2);

	//Assert
	ASSERT_ARE_EQUAL(GATEWAY_ADD_LINK_RESULT, GATEWAY_ADD_LINK_INVALID_ARG, result);

	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gw);
}


/*Tests_SRS_GATEWAY_LL_04_010: [ If the entryLink already exists it the function shall return GATEWAY_ADD_LINK_ERROR ] */
/*Tests_SRS_GATEWAY_LL_04_009: [ This function shall check if a given link already exists. ] */
TEST_FUNCTION(Gateway_LL_AddLink_DuplicatedLink_Fail)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Add another entry to the properties
	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module 2",
		"x2.dll",
		NULL
	};

	GATEWAY_LINK_ENTRY dummyLink = {
		"dummy module",
		"dummy module 2"
	};

	GATEWAY_LINK_ENTRY duplicatedLink = {
		"dummy module",
		"dummy module 2"
	};

	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_modules, &dummyEntry2, 1);
	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_links, &dummyLink, 1);

	GATEWAY_HANDLE gateway = Gateway_LL_Create(dummyProps);
	mocks.ResetAllCalls();

	//Act
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();

	GATEWAY_ADD_LINK_RESULT result = Gateway_LL_AddLink(gateway, &duplicatedLink);

	//Assert
	ASSERT_ARE_EQUAL(GATEWAY_ADD_LINK_RESULT, GATEWAY_ADD_LINK_ERROR, result);

	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gateway);
}

/*Tests_SRS_GATEWAY_LL_04_011: [If the module referenced by the entryLink->module_source or entryLink->module_sink doesn't exists this function shall return GATEWAY_ADD_LINK_ERROR ]*/
TEST_FUNCTION(Gateway_LL_AddLink_NonExistingSourceModule_Fail)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Add another entry to the properties
	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module 2",
		"x2.dll",
		NULL
	};

	GATEWAY_LINK_ENTRY dummyLink = {
		"dummy module",
		"dummy module 2"
	};

	GATEWAY_LINK_ENTRY nonExistingModuleLink = {
		"NonExisting",
		"dummy module 2"
	};

	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_modules, &dummyEntry2, 1);
	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_links, &dummyLink, 1);

	GATEWAY_HANDLE gateway = Gateway_LL_Create(dummyProps);
	mocks.ResetAllCalls();

	//Act
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();//Check link
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();//Check Module.

	GATEWAY_ADD_LINK_RESULT result = Gateway_LL_AddLink(gateway, &nonExistingModuleLink);

	//Assert
	ASSERT_ARE_EQUAL(GATEWAY_ADD_LINK_RESULT, GATEWAY_ADD_LINK_ERROR, result);

	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gateway);
}

/*Tests_SRS_GATEWAY_LL_04_011: [If the module referenced by the entryLink->module_source or entryLink->module_sink doesn't exists this function shall return GATEWAY_ADD_LINK_ERROR ]*/
TEST_FUNCTION(Gateway_LL_AddLink_NonExistingSinkModule_Fail)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Add another entry to the properties
	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module 2",
		"x2.dll",
		NULL
	};

	GATEWAY_LINK_ENTRY dummyLink = {
		"dummy module",
		"dummy module 2"
	};

	GATEWAY_LINK_ENTRY nonExistingModuleLink = {
		"dummy module 2",
		"NonExisting"
	};

	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_modules, &dummyEntry2, 1);
	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_links, &dummyLink, 1);

	GATEWAY_HANDLE gateway = Gateway_LL_Create(dummyProps);
	mocks.ResetAllCalls();

	//Act
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();//Check link
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();//Check Source Module.
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();//Check Sink Module.

	GATEWAY_ADD_LINK_RESULT result = Gateway_LL_AddLink(gateway, &nonExistingModuleLink);

	//Assert
	ASSERT_ARE_EQUAL(GATEWAY_ADD_LINK_RESULT, GATEWAY_ADD_LINK_ERROR, result);

	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gateway);
}

/*Tests_SRS_GATEWAY_LL_04_012: [ This function shall add the entryLink to the gw->links ] */
/*Tests_SRS_GATEWAY_LL_04_013: [If adding the link succeed this function shall return GATEWAY_ADD_LINK_SUCCESS]*/
TEST_FUNCTION(Gateway_LL_AddLink_Succeeds)
{
	//Arrange
	CGatewayLLMocks mocks;

	//Add another entry to the properties
	GATEWAY_MODULES_ENTRY dummyEntry2 = {
		"dummy module 2",
		"x2.dll",
		NULL
	};

	GATEWAY_LINK_ENTRY dummyLink = {
		"dummy module",
		"dummy module 2"
	};

	BASEIMPLEMENTATION::VECTOR_push_back(dummyProps->gateway_modules, &dummyEntry2, 1);

	GATEWAY_HANDLE gateway = Gateway_LL_Create(dummyProps);
	mocks.ResetAllCalls();

	//Act
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();//Check link
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();//Check Source Module.
	STRICT_EXPECTED_CALL(mocks, VECTOR_find_if(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
		.IgnoreAllArguments();//Check Sink Module.
	STRICT_EXPECTED_CALL(mocks, VECTOR_push_back(IGNORED_PTR_ARG, IGNORED_PTR_ARG, 1))
		.IgnoreArgument(1)
		.IgnoreArgument(2);

	GATEWAY_ADD_LINK_RESULT result = Gateway_LL_AddLink(gateway, &dummyLink);

	//Assert
	ASSERT_ARE_EQUAL(GATEWAY_ADD_LINK_RESULT, GATEWAY_ADD_LINK_SUCCESS, result);

	mocks.AssertActualAndExpectedCalls();

	//Cleanup
	Gateway_LL_Destroy(gateway);
}

END_TEST_SUITE(gateway_ll_unittests)
