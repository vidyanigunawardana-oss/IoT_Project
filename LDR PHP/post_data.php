<?php
    // ------------------------------------
    // 1. DATABASE CONFIGURATION
    // ------------------------------------
    $servername = "localhost"; // XAMPP MySQL server
    $username = "root";        // Default XAMPP MySQL username
    $password = "";            // Default XAMPP MySQL password (usually blank)
    $dbname = "esp32_datalogger"; 
    $table_name = "ldr_readings";   // ⬅️ Target table for LDR data

    // ------------------------------------
    // 2. CONNECT TO DATABASE
    // ------------------------------------
    $conn = new mysqli($servername, $username, $password, $dbname);

    // Check connection
    if ($conn->connect_error) {
        // Sends connection failure message back to the ESP32 Serial Monitor
        die("Connection failed: " . $conn->connect_error);
    }
    
    // ------------------------------------
    // 3. PROCESS POST DATA
    // ------------------------------------
    if ($_SERVER["REQUEST_METHOD"] == "POST") {

        // 1. Retrieve the LDR value sent by the ESP32 using the key 'ldr_value'
        $ldr_value_raw = $_POST['ldr_value']; 
        
        // 2. Sanitize the value for SQL insertion (protects against injection)
        $ldr_value_sql = $conn->real_escape_string($ldr_value_raw);

        // 3. Check if data is present and numeric
        if (empty($ldr_value_sql) || !is_numeric($ldr_value_sql)) {
            echo "Error: Invalid or non-numeric LDR data received.";
            $conn->close();
            exit();
        }

        // ------------------------------------
        // 4. PREPARE and EXECUTE SQL INSERT
        // ------------------------------------
        // The value is stored as an INT, using the sanitized string variable.
        $sql = "INSERT INTO $table_name (ldr_value) 
                VALUES ('$ldr_value_sql')";

        if ($conn->query($sql) === TRUE) {
            echo "New LDR record created successfully: Value=$ldr_value_sql";
        } else {
            echo "Error: " . $sql . "<br>" . $conn->error;
        }
    } else {
        echo "Error: Only HTTP POST requests are accepted.";
    }

    // ------------------------------------
    // 5. CLOSE CONNECTION
    // ------------------------------------
    $conn->close();
?>