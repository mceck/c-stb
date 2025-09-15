#define DS_NO_PREFIX
#include "../ds.h"
#include "../http.h"
#define JSB_IMPLEMENTATION
#include "../jsb.h"
#define JSP_IMPLEMENTATION
#include "../jsp.h"

HttpHeaders headers = {0};

int test_get() {
    log_info("Testing GET request...\n");
    HttpResponse response = {0};
    CURLcode res = http("https://jsonplaceholder.typicode.com/posts/1", &response);
    if (res != CURLE_OK) {
        log(ERROR, "HTTP request failed: %s\n", curl_easy_strerror(res));
        return 1;
    }
    log_info("HTTP status: %ld\n", response.status_code);
    log_info("Response body: %s\n", response.body.items);

    Jsp jsp = {0};
    if (jsp_init(&jsp, response.body.items, strlen(response.body.items)) != 0) {
        log(ERROR, "Failed to initialize JSP parser\n");
    }
    jsp_begin_object(&jsp);
    while (jsp_key(&jsp) == 0) {
        log_info("Key: %s\n", jsp.string);
        if (strcmp(jsp.string, "title") == 0) {
            jsp_value(&jsp);
            log_info("Post title[%d]: %s\n", jsp.type, jsp.string);
        } else if (strcmp(jsp.string, "userId") == 0) {
            jsp_value(&jsp);
            log_info("Post userId[%d]: %f\n", jsp.type, jsp.number);
        } else {
            jsp_skip(&jsp); // skip other values
            log_info("Skipped value of type %d\n", jsp.type);
        }
    }
    jsp_end_object(&jsp);
    jsp_free(&jsp);
    http_free_response(&response);
    return 0;
}

int test_post() {
    log_info("Testing POST request...\n");
    Jsb jsb = {.pp = 4}; // pretty print with 4 spaces
    jsb_begin_object(&jsb);
    {
        jsb_key(&jsb, "title");
        jsb_string(&jsb, "foo");
        jsb_key(&jsb, "body");
        jsb_string(&jsb, "bar");
        jsb_key(&jsb, "userId");
        jsb_int(&jsb, 1);
        jsb_key(&jsb, "nest");
        jsb_begin_object(&jsb);
        {
            jsb_key(&jsb, "nested");
            jsb_bool(&jsb, true);
            jsb_key(&jsb, "array");
            jsb_begin_array(&jsb);
            {
                jsb_begin_object(&jsb);
                {
                    jsb_key(&jsb, "a");
                    jsb_int(&jsb, 1);
                    jsb_key(&jsb, "b");
                    jsb_string(&jsb, "two");
                }
                jsb_end_object(&jsb);
            }
            jsb_end_array(&jsb);
        }
        jsb_end_object(&jsb);
    }
    jsb_end_object(&jsb);

    log_info("POST body: %s\n", jsb_get(&jsb));

    HttpResponse response = {0};
    CURLcode res = http("https://jsonplaceholder.typicode.com/posts", &response, .method = HTTP_POST, .headers = &headers, .body = jsb_get(&jsb));
    jsb_free(&jsb);
    if (res != CURLE_OK) {
        log(ERROR, "HTTP request failed: %s\n", curl_easy_strerror(res));
        return 1;
    }
    log_info("HTTP status: %ld\n", response.status_code);
    log_info("Response body: %s\n", response.body.items);

    Jsp jsp = {0};
    if (jsp_init(&jsp, response.body.items, strlen(response.body.items)) != 0) {
        log(ERROR, "Failed to initialize JSP parser\n");
    }
    jsp_begin_object(&jsp);
    while (jsp_key(&jsp) == 0) {
        log_info("Key: %s\n", jsp.string);
        if (strcmp(jsp.string, "title") == 0) {
            jsp_value(&jsp);
            log_info("Post title[%d]: %s\n", jsp.type, jsp.string);
        } else if (strcmp(jsp.string, "userId") == 0) {
            jsp_value(&jsp);
            log_info("Post userId[%d]: %f\n", jsp.type, jsp.number);
        } else if (strcmp(jsp.string, "nest") == 0) {
            jsp_begin_object(&jsp);
            while (jsp_key(&jsp) == 0) {
                log_info("  Key: %s\n", jsp.string);
                if (strcmp(jsp.string, "nested") == 0) {
                    jsp_value(&jsp);
                    log_info("  nested[%d]: %d\n", jsp.type, jsp.boolean);
                } else if (strcmp(jsp.string, "array") == 0) {
                    jsp_begin_array(&jsp);
                    while (jsp_begin_object(&jsp) == 0) {
                        while (jsp_key(&jsp) == 0) {
                            log_info("    Key: %s\n", jsp.string);
                            if (strcmp(jsp.string, "a") == 0) {
                                jsp_value(&jsp);
                                log_info("    a[%d]: %f\n", jsp.type, jsp.number);
                            } else if (strcmp(jsp.string, "b") == 0) {
                                jsp_value(&jsp);
                                log_info("    b[%d]: %s\n", jsp.type, jsp.string);
                            } else {
                                jsp_value(&jsp); // skip other values
                                log_info("    Skipped value of type %d\n", jsp.type);
                            }
                        }
                        jsp_end_object(&jsp);
                    }
                    jsp_end_array(&jsp);
                } else {
                    jsp_skip(&jsp); // skip other values
                    log_info("  Skipped value of type %d\n", jsp.type);
                }
            }
        } else {
            jsp_skip(&jsp); // skip other values
            log_info("Skipped value of type %d\n", jsp.type);
        }
    }
    jsp_end_object(&jsp);
    jsp_free(&jsp);

    http_free_response(&response);
    return 0;
}

int test_put() {
    log_info("Testing PUT request...\n");
    Jsb jsb = {.pp = 4}; // pretty print with 4 spaces
    jsb_begin_object(&jsb);
    {
        jsb_key(&jsb, "id");
        jsb_int(&jsb, 1);
        jsb_key(&jsb, "title");
        jsb_string(&jsb, "foo");
        jsb_key(&jsb, "body");
        jsb_string(&jsb, "bar");
        jsb_key(&jsb, "userId");
        jsb_int(&jsb, 1);
    }
    jsb_end_object(&jsb);

    log_info("PUT body: %s\n", jsb_get(&jsb));

    HttpResponse response = {0};
    CURLcode res = http("https://jsonplaceholder.typicode.com/posts/1", &response, .method = HTTP_PUT, .headers = &headers, .body = jsb_get(&jsb));
    if (res != CURLE_OK) {
        log(ERROR, "HTTP request failed: %s\n", curl_easy_strerror(res));
        jsb_free(&jsb);
        return 1;
    }
    log_info("HTTP status: %ld\n", response.status_code);
    log_info("Response body: %s\n", response.body.items);

    jsb_free(&jsb);
    http_free_response(&response);
    return 0;
}

int test_patch() {
    log_info("Testing PATCH request...\n");
    Jsb jsb = {.pp = 4}; // pretty print with 4 spaces
    jsb_begin_object(&jsb);
    {
        jsb_key(&jsb, "title");
        jsb_string(&jsb, "foo updated");
    }
    jsb_end_object(&jsb);

    log_info("PATCH body: %s\n", jsb_get(&jsb));

    HttpResponse response = {0};
    CURLcode res = http("https://jsonplaceholder.typicode.com/posts/1", &response, .method = HTTP_PATCH, .headers = &headers, .body = jsb_get(&jsb));
    if (res != CURLE_OK) {
        log(ERROR, "HTTP request failed: %s\n", curl_easy_strerror(res));
        jsb_free(&jsb);
        return 1;
    }
    log_info("HTTP status: %ld\n", response.status_code);
    log_info("Response body: %s\n", response.body.items);

    jsb_free(&jsb);
    http_free_response(&response);
    return 0;
}

int test_delete() {
    log_info("Testing DELETE request...\n");
    HttpResponse response = {0};
    CURLcode res = http("https://jsonplaceholder.typicode.com/posts/1", &response, .method = HTTP_DELETE);
    if (res != CURLE_OK) {
        log(ERROR, "HTTP request failed: %s\n", curl_easy_strerror(res));
        return 1;
    }
    log_info("HTTP status: %ld\n", response.status_code);
    log_info("Response body: %s\n", response.body.items);

    http_free_response(&response);
    return 0;
}

#define LOG_TEST r = r ||
int test_jsb_builder() {
    log_info("Testing JSB builder...\n");
    Jsb jsb = {.pp = 4}; // pretty print with 4 spaces
    int r = 0;
    LOG_TEST jsb_begin_object(&jsb);
    {
        LOG_TEST jsb_key(&jsb, "message");
        LOG_TEST jsb_string(&jsb, "Hello, World!");
        LOG_TEST jsb_key(&jsb, "data");
        LOG_TEST jsb_begin_array(&jsb);
        {
            LOG_TEST jsb_string(&jsb, "item1");
            LOG_TEST jsb_int(&jsb, 2);
            LOG_TEST jsb_number(&jsb, 2.432, 2);
            LOG_TEST jsb_bool(&jsb, true);
            LOG_TEST jsb_null(&jsb);
            LOG_TEST jsb_begin_object(&jsb);
            {
                LOG_TEST jsb_key(&jsb, "key1");
                LOG_TEST jsb_string(&jsb, "value1");
            }
            LOG_TEST jsb_end_object(&jsb);
        }
        LOG_TEST jsb_end_array(&jsb);
    }
    LOG_TEST jsb_end_object(&jsb);
    log_info("JSB: %s\n", jsb_get(&jsb));
    jsb_free(&jsb);

    if (r) {
        log(ERROR, "JSB builder test failed\n");
        return 1;
    }
    return 0;
}

int test_jsp_j1() {
    StringBuilder sb = {0};
    if (!read_entire_file("tests/json/j1.json", &sb)) {
        log(ERROR, "Failed to read j1.json\n");
        return 1;
    }
    log_info("Testing JSON parser with input: %s\n", sb.items);
    Jsp jsp = {0};
    jsp_sinit(&jsp, sb.items);
    int r = 0;
    LOG_TEST jsp_begin_object(&jsp);
    while (jsp_key(&jsp) == 0) {
        if (strcmp(jsp.string, "name") == 0) {
            LOG_TEST jsp_value(&jsp);
            printf("Name: %s\n", jsp.string);
        } else if (strcmp(jsp.string, "age") == 0) {
            LOG_TEST jsp_value(&jsp);
            printf("Age: %.0f\n", jsp.number);
        } else if (strcmp(jsp.string, "is_student") == 0) {
            LOG_TEST jsp_value(&jsp);
            printf("Is student: %s\n", jsp.boolean ? "true" : "false");
        } else if (strcmp(jsp.string, "array") == 0) {
            LOG_TEST jsp_begin_array(&jsp);
            while (jsp_value(&jsp) == 0) {
                if (jsp.type == JSP_TYPE_STRING) {
                    printf("Array item (string): %s\n", jsp.string);
                } else if (jsp.type == JSP_TYPE_NUMBER) {
                    printf("Array item (number): %.2f\n", jsp.number);
                } else if (jsp.type == JSP_TYPE_BOOLEAN) {
                    printf("Array item (boolean): %s\n", jsp.boolean ? "true" : "false");
                }
            }
            LOG_TEST jsp_end_array(&jsp);
        } else {
            LOG_TEST jsp_skip(&jsp); // skip other values
        }
    }
    LOG_TEST jsp_end_object(&jsp);

    jsp_free(&jsp);
    da_free(&sb);
    if (r) {
        log(ERROR, "j1.json structure invalid\n");
        return 1;
    }
    log_info("j1.json structure validated\n");
    return 0;
}

int test_jsp_j2() {
    log_info("Testing JSON parser with j2.json\n");
    StringBuilder sb = {0};
    if (!read_entire_file("tests/json/j2.json", &sb)) {
        log(ERROR, "Failed to read j2.json\n");
        return 1;
    }
    // log_info("Testing JSON parser with survey data: %s\n", sb.items);

    Jsp jsp = {0};
    jsp_sinit(&jsp, sb.items);
    int r = 0;
    // Parse root object
    LOG_TEST jsp_begin_object(&jsp);
    while (jsp_key(&jsp) == 0) {
        if (strcmp(jsp.string, "survey_results") == 0) {
            // Parse survey_results object
            LOG_TEST jsp_begin_object(&jsp);
            while (jsp_key(&jsp) == 0) {
                if (strcmp(jsp.string, "survey_id") == 0) {
                    LOG_TEST jsp_value(&jsp);
                    printf("Survey ID: %s\n", jsp.string);

                } else if (strcmp(jsp.string, "responses") == 0) {
                    // Parse responses array
                    LOG_TEST jsp_begin_array(&jsp);
                    int response_count = 0;
                    while (jsp_begin_object(&jsp) == 0) {
                        printf("\n--- Response %d ---\n", ++response_count);
                        while (jsp_key(&jsp) == 0) {
                            if (strcmp(jsp.string, "respondent_id") == 0) {
                                LOG_TEST jsp_value(&jsp);
                                printf("Respondent ID: %s\n", jsp.string);

                            } else if (strcmp(jsp.string, "answers") == 0) {
                                // Parse answers object
                                LOG_TEST jsp_begin_object(&jsp);
                                while (jsp_key(&jsp) == 0) {
                                    char question[10];
                                    strcpy(question, jsp.string);
                                    // Try to parse as value first
                                    if (jsp_value(&jsp) == 0) {
                                        if (jsp.type == JSP_TYPE_STRING) {
                                            printf("  %s: %s\n", question, jsp.string);
                                        } else if (jsp.type == JSP_TYPE_NUMBER) {
                                            printf("  %s: %.2f\n", question, jsp.number);
                                        } else if (jsp.type == JSP_TYPE_NULL) {
                                            printf("  %s: null\n", question);
                                        }
                                    } else if (jsp_begin_array(&jsp) == 0) {
                                        // It's an array
                                        printf("  %s: [", question);
                                        int first = 1;
                                        while (jsp_value(&jsp) == 0) {
                                            if (!first) printf(", ");
                                            first = 0;
                                            if (jsp.type == JSP_TYPE_STRING) {
                                                printf("\"%s\"", jsp.string);
                                            } else if (jsp.type == JSP_TYPE_NUMBER) {
                                                printf("%.0f", jsp.number);
                                            } else if (jsp.type == JSP_TYPE_BOOLEAN) {
                                                printf("%s", jsp.boolean ? "true" : "false");
                                            }
                                        }
                                        LOG_TEST jsp_end_array(&jsp);
                                        printf("]\n");
                                    } else if (jsp_begin_object(&jsp) == 0) {
                                        // It's an object
                                        printf("  %s: {\n", question);
                                        while (jsp_key(&jsp) == 0) {
                                            char subkey[20];
                                            strcpy(subkey, jsp.string);
                                            jsp_value(&jsp);
                                            if (jsp.type == JSP_TYPE_NUMBER) {
                                                printf("    %s: %.0f\n", subkey, jsp.number);
                                            } else if (jsp.type == JSP_TYPE_STRING) {
                                                printf("    %s: %s\n", subkey, jsp.string);
                                            } else if (jsp.type == JSP_TYPE_BOOLEAN) {
                                                printf("    %s: %s\n", subkey, jsp.boolean ? "true" : "false");
                                            }
                                        }
                                        LOG_TEST jsp_end_object(&jsp);
                                        printf("  }\n");
                                    }
                                }
                                LOG_TEST jsp_end_object(&jsp);

                            } else if (strcmp(jsp.string, "completion_time") == 0) {
                                LOG_TEST jsp_value(&jsp);
                                if (jsp.type == JSP_TYPE_NUMBER) {
                                    printf("Completion time: %.1f seconds\n", jsp.number);
                                } else if (jsp.type == JSP_TYPE_STRING) {
                                    printf("Completion time: %s\n", jsp.string);
                                }

                            } else if (strcmp(jsp.string, "device_info") == 0) {
                                if (jsp_value(&jsp) == 0) {
                                    // It's a string
                                    printf("Device: %s\n", jsp.string);
                                } else if (jsp_begin_object(&jsp) == 0) {
                                    // It's an object
                                    printf("Device info: {\n");
                                    while (jsp_key(&jsp) == 0) {
                                        char device_key[20];
                                        strcpy(device_key, jsp.string);
                                        LOG_TEST jsp_value(&jsp);
                                        printf("  %s: %s\n", device_key, jsp.string);
                                    }
                                    LOG_TEST jsp_end_object(&jsp);
                                    printf("}\n");
                                }
                            } else {
                                jsp_skip(&jsp); // skip other values
                            }
                        }
                        LOG_TEST jsp_end_object(&jsp);
                    }
                    LOG_TEST jsp_end_array(&jsp);

                } else if (strcmp(jsp.string, "analytics") == 0) {
                    printf("\n--- Analytics ---\n");
                    LOG_TEST jsp_begin_object(&jsp);
                    while (jsp_key(&jsp) == 0) {
                        if (strcmp(jsp.string, "avg_completion_time") == 0) {
                            LOG_TEST jsp_value(&jsp);
                            printf("Average completion time: %.2f\n", jsp.number);

                        } else if (strcmp(jsp.string, "response_rates") == 0) {
                            printf("Response rates:\n");
                            LOG_TEST jsp_begin_object(&jsp);
                            while (jsp_key(&jsp) == 0) {
                                char rate_key[10];
                                strcpy(rate_key, jsp.string);

                                if (jsp_value(&jsp) == 0) {
                                    if (jsp.type == JSP_TYPE_STRING) {
                                        printf("  %s: %s\n", rate_key, jsp.string);
                                    } else if (jsp.type == JSP_TYPE_NUMBER) {
                                        printf("  %s: %.3f\n", rate_key, jsp.number);
                                    }
                                } else if (jsp_begin_array(&jsp) == 0) {
                                    printf("  %s: [", rate_key);
                                    int first = 1;
                                    while (jsp_value(&jsp) == 0) {
                                        if (!first) printf(", ");
                                        first = 0;
                                        printf("%.2f", jsp.number);
                                    }
                                    LOG_TEST jsp_end_array(&jsp);
                                    printf("]\n");
                                } else if (jsp_begin_object(&jsp) == 0) {
                                    printf("  %s: {", rate_key);
                                    int first = 1;
                                    while (jsp_key(&jsp) == 0) {
                                        if (!first) printf(", ");
                                        first = 0;
                                        char obj_key[20];
                                        strcpy(obj_key, jsp.string);
                                        jsp_value(&jsp);
                                        printf("%s: %.0f", obj_key, jsp.number);
                                    }
                                    LOG_TEST jsp_end_object(&jsp);
                                    printf("}\n");
                                }
                            }
                            LOG_TEST jsp_end_object(&jsp);

                        } else if (strcmp(jsp.string, "demographics") == 0) {
                            printf("Demographics:\n");
                            LOG_TEST jsp_begin_array(&jsp);
                            while (jsp_begin_array(&jsp) == 0) {
                                char demo_type[20] = "", demo_value[20] = "";
                                double demo_count = 0;

                                // First element (type)
                                if (jsp_value(&jsp) == 0 && jsp.type == JSP_TYPE_STRING) {
                                    strcpy(demo_type, jsp.string);
                                }
                                // Second element (value)
                                if (jsp_value(&jsp) == 0 && jsp.type == JSP_TYPE_STRING) {
                                    strcpy(demo_value, jsp.string);
                                }
                                // Third element (count or object)
                                if (jsp_value(&jsp) == 0) {
                                    if (jsp.type == JSP_TYPE_NUMBER) {
                                        demo_count = jsp.number;
                                        printf("  %s %s: %.0f\n", demo_type, demo_value, demo_count);
                                    }
                                } else if (jsp_begin_object(&jsp) == 0) {
                                    printf("  %s %s: {", demo_type, demo_value);
                                    int first = 1;
                                    while (jsp_key(&jsp) == 0) {
                                        if (!first) printf(", ");
                                        first = 0;
                                        char obj_key[20];
                                        strcpy(obj_key, jsp.string);
                                        LOG_TEST jsp_value(&jsp);
                                        if (jsp.type == JSP_TYPE_NUMBER) {
                                            printf("%s: %.1f", obj_key, jsp.number);
                                        }
                                    }
                                    LOG_TEST jsp_end_object(&jsp);
                                    printf("}\n");
                                }
                                LOG_TEST jsp_end_array(&jsp);
                            }
                            LOG_TEST jsp_end_array(&jsp);
                        } else {
                            LOG_TEST jsp_value(&jsp);
                        }
                    }
                    LOG_TEST jsp_end_object(&jsp);
                } else {
                    // Skip unknown survey_results key
                    LOG_TEST jsp_skip(&jsp);
                }
            }
            LOG_TEST jsp_end_object(&jsp);
        } else {
            // Skip unknown root key
            LOG_TEST jsp_skip(&jsp);
        }
    }
    LOG_TEST jsp_end_object(&jsp);

    jsp_free(&jsp);
    da_free(&sb);
    if (r) {
        log(ERROR, "j2.json structure invalid\n");
        return 1;
    }
    log_info("j2.json structure validated\n");
    return 0;
}

int test_jsp_j3() {
    log_info("Testing JSON parser with j3.json\n");
    StringBuilder sb = {0};
    if (!read_entire_file("tests/json/j3.json", &sb)) {
        log(ERROR, "Failed to read j3.json\n");
        return 1;
    }
    // log_info("Testing JSON parser with complex data: %s\n", sb.items);
    int r = 0;
    Jsp jsp = {0};
    jsp_sinit(&jsp, sb.items);

    // Parse root object
    LOG_TEST jsp_begin_object(&jsp);
    while (jsp_key(&jsp) == 0) {
        if (strcmp(jsp.string, "metadata") == 0) {
            printf("=== METADATA ===\n");
            LOG_TEST jsp_begin_object(&jsp);
            while (jsp_key(&jsp) == 0) {
                char meta_key[30];
                strcpy(meta_key, jsp.string);
                LOG_TEST jsp_value(&jsp);
                printf("%s: %s\n", meta_key, jsp.string);
            }
            jsp_end_object(&jsp);

        } else if (strcmp(jsp.string, "data") == 0) {
            printf("\n=== DATA ===\n");
            LOG_TEST jsp_begin_object(&jsp);
            while (jsp_key(&jsp) == 0) {
                if (strcmp(jsp.string, "users") == 0) {
                    printf("\n--- Users ---\n");
                    LOG_TEST jsp_begin_array(&jsp);
                    int user_count = 0;
                    while (jsp_begin_object(&jsp) == 0) {
                        printf("\nUser %d:\n", ++user_count);
                        while (jsp_key(&jsp) == 0) {
                            if (strcmp(jsp.string, "id") == 0) {
                                LOG_TEST jsp_value(&jsp);
                                printf("  ID: %.0f\n", jsp.number);

                            } else if (strcmp(jsp.string, "name") == 0) {
                                LOG_TEST jsp_value(&jsp);
                                printf("  Name: %s\n", jsp.string);

                            } else if (strcmp(jsp.string, "email") == 0) {
                                LOG_TEST jsp_value(&jsp);
                                printf("  Email: %s\n", jsp.string);

                            } else if (strcmp(jsp.string, "preferences") == 0) {
                                printf("  Preferences:\n");
                                LOG_TEST jsp_begin_object(&jsp);
                                while (jsp_key(&jsp) == 0) {
                                    if (strcmp(jsp.string, "theme") == 0) {
                                        LOG_TEST jsp_value(&jsp);
                                        printf("    Theme: %s\n", jsp.string);

                                    } else if (strcmp(jsp.string, "notifications") == 0) {
                                        printf("    Notifications:\n");
                                        LOG_TEST jsp_begin_object(&jsp);
                                        while (jsp_key(&jsp) == 0) {
                                            char notif_key[20];
                                            strcpy(notif_key, jsp.string);

                                            if (strcmp(notif_key, "frequency") == 0) {
                                                printf("      Frequency:\n");
                                                LOG_TEST jsp_begin_object(&jsp);
                                                while (jsp_key(&jsp) == 0) {
                                                    char time_key[15];
                                                    strcpy(time_key, jsp.string);
                                                    printf("        %s: [", time_key);
                                                    LOG_TEST jsp_begin_array(&jsp);
                                                    int first = 1;
                                                    while (jsp_value(&jsp) == 0) {
                                                        if (!first) printf(", ");
                                                        first = 0;
                                                        printf("%.0f", jsp.number);
                                                    }
                                                    LOG_TEST jsp_end_array(&jsp);
                                                    printf("]\n");
                                                }
                                                jsp_end_object(&jsp);
                                            } else {
                                                LOG_TEST jsp_value(&jsp);
                                                if (jsp.type == JSP_TYPE_BOOLEAN) {
                                                    printf("      %s: %s\n", notif_key, jsp.boolean ? "true" : "false");
                                                } else if (jsp.type == JSP_TYPE_NULL) {
                                                    printf("      %s: null\n", notif_key);
                                                }
                                            }
                                        }
                                        LOG_TEST jsp_end_object(&jsp);
                                    } else {
                                        // Skip other preference keys
                                        jsp_skip(&jsp);
                                    }
                                }
                                LOG_TEST jsp_end_object(&jsp);

                            } else if (strcmp(jsp.string, "activity_log") == 0) {
                                printf("  Activity Log:\n");
                                LOG_TEST jsp_begin_array(&jsp);
                                int activity_count = 0;
                                while (jsp_begin_object(&jsp) == 0) {
                                    printf("    Activity %d:\n", ++activity_count);
                                    while (jsp_key(&jsp) == 0) {
                                        if (strcmp(jsp.string, "action") == 0) {
                                            LOG_TEST jsp_value(&jsp);
                                            printf("      Action: %s\n", jsp.string);

                                        } else if (strcmp(jsp.string, "timestamp") == 0) {
                                            LOG_TEST jsp_value(&jsp);
                                            printf("      Timestamp: %s\n", jsp.string);

                                        } else if (strcmp(jsp.string, "metadata") == 0) {
                                            printf("      Metadata:\n");
                                            LOG_TEST jsp_begin_object(&jsp);
                                            while (jsp_key(&jsp) == 0) {
                                                char meta_key[20];
                                                strcpy(meta_key, jsp.string);
                                                LOG_TEST jsp_value(&jsp);
                                                printf("        %s: %s\n", meta_key, jsp.string);
                                            }
                                            LOG_TEST jsp_end_object(&jsp);

                                        } else if (strcmp(jsp.string, "page_data") == 0) {
                                            printf("      Page Data:\n");
                                            LOG_TEST jsp_begin_object(&jsp);
                                            while (jsp_key(&jsp) == 0) {
                                                if (strcmp(jsp.string, "url") == 0) {
                                                    LOG_TEST jsp_value(&jsp);
                                                    printf("        URL: %s\n", jsp.string);

                                                } else if (strcmp(jsp.string, "load_time") == 0) {
                                                    LOG_TEST jsp_value(&jsp);
                                                    printf("        Load Time: %.3f\n", jsp.number);

                                                } else if (strcmp(jsp.string, "interactions") == 0) {
                                                    printf("        Interactions:\n");
                                                    LOG_TEST jsp_begin_array(&jsp);
                                                    int interaction_count = 0;
                                                    while (jsp_begin_object(&jsp) == 0) {
                                                        printf("          Interaction %d:\n", ++interaction_count);
                                                        while (jsp_key(&jsp) == 0) {
                                                            char interact_key[15];
                                                            strcpy(interact_key, jsp.string);
                                                            LOG_TEST jsp_value(&jsp);
                                                            if (jsp.type == JSP_TYPE_STRING) {
                                                                printf("            %s: %s\n", interact_key, jsp.string);
                                                            } else if (jsp.type == JSP_TYPE_NUMBER) {
                                                                printf("            %s: %.1f\n", interact_key, jsp.number);
                                                            }
                                                        }
                                                        LOG_TEST jsp_end_object(&jsp);
                                                    }
                                                    LOG_TEST jsp_end_array(&jsp);
                                                } else {
                                                    // Skip other page_data keys
                                                    LOG_TEST jsp_skip(&jsp);
                                                }
                                            }
                                            LOG_TEST jsp_end_object(&jsp);
                                        } else {
                                            // Skip other activity keys
                                            jsp_skip(&jsp);
                                        }
                                    }
                                    LOG_TEST jsp_end_object(&jsp);
                                }
                                LOG_TEST jsp_end_array(&jsp);
                            } else {
                                // Skip other user keys
                                jsp_skip(&jsp);
                            }
                        }
                        LOG_TEST jsp_end_object(&jsp);
                    }
                    LOG_TEST jsp_end_array(&jsp);

                } else if (strcmp(jsp.string, "system_config") == 0) {
                    printf("\n--- System Config ---\n");
                    LOG_TEST jsp_begin_object(&jsp);
                    while (jsp_key(&jsp) == 0) {
                        if (strcmp(jsp.string, "database") == 0) {
                            printf("Database:\n");
                            LOG_TEST jsp_begin_object(&jsp);
                            while (jsp_key(&jsp) == 0) {
                                if (strcmp(jsp.string, "host") == 0) {
                                    LOG_TEST jsp_value(&jsp);
                                    printf("  Host: %s\n", jsp.string);

                                } else if (strcmp(jsp.string, "port") == 0) {
                                    LOG_TEST jsp_value(&jsp);
                                    printf("  Port: %.0f\n", jsp.number);

                                } else if (strcmp(jsp.string, "credentials") == 0) {
                                    printf("  Credentials:\n");
                                    LOG_TEST jsp_begin_object(&jsp);
                                    while (jsp_key(&jsp) == 0) {
                                        char cred_key[15];
                                        strcpy(cred_key, jsp.string);
                                        LOG_TEST jsp_value(&jsp);
                                        if (jsp.type == JSP_TYPE_BOOLEAN) {
                                            printf("    %s: %s\n", cred_key, jsp.boolean ? "true" : "false");
                                        } else if (jsp.type == JSP_TYPE_STRING) {
                                            printf("    %s: %s\n", cred_key, jsp.string);
                                        }
                                    }
                                    LOG_TEST jsp_end_object(&jsp);
                                } else {
                                    LOG_TEST jsp_value(&jsp);
                                }
                            }
                            LOG_TEST jsp_end_object(&jsp);

                        } else if (strcmp(jsp.string, "api_endpoints") == 0) {
                            printf("API Endpoints:\n");
                            LOG_TEST jsp_begin_object(&jsp);
                            while (jsp_key(&jsp) == 0) {
                                char endpoint[50];
                                strcpy(endpoint, jsp.string);
                                printf("  %s:\n", endpoint);
                                LOG_TEST jsp_begin_object(&jsp);
                                while (jsp_key(&jsp) == 0) {
                                    char config_key[20];
                                    strcpy(config_key, jsp.string);

                                    if (strcmp(config_key, "methods") == 0) {
                                        printf("    Methods: [");
                                        LOG_TEST jsp_begin_array(&jsp);
                                        int first = 1;
                                        while (jsp_value(&jsp) == 0) {
                                            if (!first) printf(", ");
                                            first = 0;
                                            printf("%s", jsp.string);
                                        }
                                        LOG_TEST jsp_end_array(&jsp);
                                        printf("]\n");
                                    } else {
                                        LOG_TEST jsp_value(&jsp);
                                        if (jsp.type == JSP_TYPE_NUMBER) {
                                            printf("    %s: %.0f\n", config_key, jsp.number);
                                        } else if (jsp.type == JSP_TYPE_STRING) {
                                            printf("    %s: %s\n", config_key, jsp.string);
                                        } else if (jsp.type == JSP_TYPE_BOOLEAN) {
                                            printf("    %s: %s\n", config_key, jsp.boolean ? "true" : "false");
                                        }
                                    }
                                }
                                LOG_TEST jsp_end_object(&jsp);
                            }
                            LOG_TEST jsp_end_object(&jsp);
                        } else {
                            // Skip other system_config keys
                            jsp_skip(&jsp);
                        }
                    }
                    LOG_TEST jsp_end_object(&jsp);
                } else {
                    // Skip other data keys
                    jsp_skip(&jsp);
                }
            }
            LOG_TEST jsp_end_object(&jsp);
        } else {
            // Skip other root keys
            jsp_skip(&jsp);
        }
    }
    LOG_TEST jsp_end_object(&jsp);

    jsp_free(&jsp);
    da_free(&sb);
    if (r) {
        log(ERROR, "j3.json structure invalid\n");
        return 1;
    }
    log_info("j3.json structure validated\n");
    return 0;
}

int main() {
    http_init();
    da_append(&headers, "Content-Type: application/json");
    int r = 0;
    log_info("--------------------------------------------------\n");
    LOG_TEST test_jsb_builder();
    log_info("--------------------------------------------------\n");
    LOG_TEST test_jsp_j1();
    log_info("--------------------------------------------------\n");
    LOG_TEST test_jsp_j2();
    log_info("--------------------------------------------------\n");
    LOG_TEST test_jsp_j3();
    log_info("--------------------------------------------------\n");
    LOG_TEST test_get();
    log_info("--------------------------------------------------\n");
    LOG_TEST test_post();
    log_info("--------------------------------------------------\n");
    LOG_TEST test_put();
    log_info("--------------------------------------------------\n");
    LOG_TEST test_patch();
    log_info("--------------------------------------------------\n");
    LOG_TEST test_delete();

    da_free(&headers);
    http_cleanup();

    if (r) {
        log(ERROR, "Some tests failed\n");
        return 1;
    }
    log_info("All tests passed successfully\n");
    return 0;
}