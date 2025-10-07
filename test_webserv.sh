#!/bin/bash

# Define the base URL for the server
BASE_URL="http://localhost:8080"

# Function to print the result of each test
print_result() {
    local test_name="$1"
    local status="$2"
    if [ "$status" == "PASS" ]; then
        echo -e "\e[32m$test_name: $status\e[0m" # Green for PASS
    else
        echo -e "\e[31m$test_name: $status\e[0m" # Red for FAIL
    fi
}

# Test 1: GET /
echo "Running Test 1: GET /"
if curl -X GET "$BASE_URL/" -s -o /dev/null -w "%{http_code}" | grep -q "200"; then
    print_result "Test 1" "PASS"
else
    print_result "Test 1" "FAIL"
fi

# Test 2: GET /index.html
echo "Running Test 2: GET /index.html"
if curl -X GET "$BASE_URL/index.html" -s -o /dev/null -w "%{http_code}" | grep -q "200"; then
    print_result "Test 2" "PASS"
else
    print_result "Test 2" "FAIL"
fi

# Test 3: GET /nonexistent.html
echo "Running Test 3: GET /nonexistent.html"
if curl -v GET "$BASE_URL/nonexistent.html" -s -o /dev/null -w "%{http_code}" | grep -q "404"; then
    print_result "Test 3" "PASS"
else
    print_result "Test 3" "FAIL"
fi

# Test 4: GET /var/www/html/
echo "Running Test 4: GET /var/www/html/"
if curl -X GET "$BASE_URL/var/www/html/" -s -o /dev/null -w "%{http_code}" | grep -q "200"; then
    print_result "Test 4" "PASS"
else
    print_result "Test 4" "FAIL"
fi

# Test 5: GET /images/01.png
echo "Running Test 5: GET /images/01.png"
if curl -v GET "$BASE_URL/images/01.png" -s -o /dev/null -w "%{http_code}" | grep -q "200"; then
    print_result "Test 5" "PASS"
else
    print_result "Test 5" "FAIL"
fi

# Test 6: GET /uploads/
echo "Running Test 6: GET /uploads/"
if curl -v GET "$BASE_URL/uploads/" -s -o /dev/null -w "%{http_code}" | grep -q "200"; then
    print_result "Test 6" "PASS"
else
    print_result "Test 6" "FAIL"
fi

# Test 7: GET /cgi-bin/test_cgi.py
echo "Running Test 7: GET /cgi-bin/test_cgi.py"
if curl -v GET "$BASE_URL/cgi-bin/test_cgi.py" -s -o /dev/null -w "%{http_code}" | grep -q "200"; then
    print_result "Test 7" "PASS"
else
    print_result "Test 7" "FAIL"
fi


echo "Running Test 8: GET /cgi-bin/test_cgi.py\?msg=hey"
if curl -v  "$BASE_URL/cgi-bin/test_cgi.py?msg=hey" -s -o /dev/null -w "%{http_code}" | grep -q "200"; then
    print_result "Test 8" "PASS"
else
    print_result "Test 8" "FAIL"
fi

# Test 9: GET /error.html
echo "Running Test 9: GET /error.html"
if curl -X GET "$BASE_URL/error.html" -s -o /dev/null -w "%{http_code}" | grep -q "200"; then
    print_result "Test 9" "PASS"
else
    print_result "Test 9" "FAIL"
fi

# Test 9: GET /upload.html
echo "Running Test 10: GET /upload.html"
if curl -X GET "$BASE_URL/upload.html" -s -o /dev/null -w "%{http_code}" | grep -q "200"; then
    print_result "Test 10" "PASS"
else
    print_result "Test 10" "FAIL"
fi

# Test 10: GET /browser_test.html
echo "Running Test 11: GET /browser_test.html"
if curl -X GET "$BASE_URL/browser_test.html" -s -o /dev/null -w "%{http_code}" | grep -q "200"; then
    print_result "Test 11" "PASS"
else
    print_result "Test 11" "FAIL"
fi

# --- POST TESTS START ---



# Print completion message
echo "All tests completed."