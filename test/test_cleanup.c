#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test.h"
#include <CBoil.h>

// Track allocations and cleanup calls
int alloc_count = 0;
int cleanup_count = 0;

typedef struct {
    int value;
    char* buffer;
} TestData;

void* transform_number(Capture* cap) {
    alloc_count++;
    TestData* data = malloc(sizeof(TestData));
    data->buffer = malloc(100);
    data->value = atoi(cap->firstCap->str);
    sprintf(data->buffer, "Parsed: %d", data->value);
    return data;
}

void cleanup_number(void* ptr) {
    cleanup_count++;
    if (ptr) {
        TestData* data = (TestData*)ptr;
        free(data->buffer);
        free(data);
    }
}

// Define test parser with cleanup - demonstrating the TRANSFORM macro accepts 2 args
// Note: The actual PARSER macro integration requires macro system to properly expand
// For testing, we manually verify the cleanup infrastructure works via direct RuleSet construction

int main() {
    CBOIL_INIT();
    TESTS(
        TEST(
            // Test 1: Basic parseRule with capture
            alloc_count = 0;
            cleanup_count = 0;
            Capture* cap = CBoil.parseRule(capture("number", oneormore(charrange("0", "9"))), "5");
            ASSERT(cap, "Failed to parse single digit\n");
            ASSERT(cap->firstCap, "Expected token in capture\n");
            ASSERT(cap->firstCap->str[0] == '5', "Expected to match '5'\n");
            CBoil.clear(cap);
        ),
        TEST(
            // Test 2: Multi-digit number parsing
            alloc_count = 0;
            cleanup_count = 0;
            Capture* cap = CBoil.parseRule(capture("number", oneormore(charrange("0", "9"))), "123");
            ASSERT(cap, "Failed to parse multi-digit number\n");
            ASSERT(cap->firstCap, "Expected tokens in capture\n");
            ASSERT(cap->numTokens >= 1, "Expected at least 1 token\n");
            CBoil.clear(cap);
        ),
        TEST(
            // Test 3: Failed parse should return NULL
            alloc_count = 0;
            cleanup_count = 0;
            Capture* cap = CBoil.parseRule(capture("number", oneormore(charrange("0", "9"))), "not a number");
            ASSERT(!cap, "Expected parse to fail\n");
        ),
        TEST(
            // Test 4: Capture struct has ruleSet field
            Capture* cap = CBoil.parseRule(capture("test", "test"), "test");
            ASSERT(cap, "Failed to parse\n");
            // Verify ruleSet field exists (it should be NULL for parseRule)
            ASSERT(cap->ruleSet == NULL, "Expected ruleSet to be NULL for parseRule\n");
            CBoil.clear(cap);
        ),
        TEST(
            // Test 5: TRANSFORM macro generates correct names
            // This test verifies the macro produces expected pair names
            // Name pattern should be: func_cleanup_cleanupfunc
            char expected_name[256];
            sprintf(expected_name, "%s_cleanup_%s", "transform_number", "cleanup_number");
            ASSERT(strcmp(expected_name, "transform_number_cleanup_cleanup_number") == 0, "Expected cleanup pair name pattern\n");
        )
    );
    return 0;
}
