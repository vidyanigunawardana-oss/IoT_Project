<?php
    // ------------------------------------
    // 1. DATABASE CONFIGURATION
    // ------------------------------------
    $servername = "localhost"; 
    $username = "root";        
    $password = "";            
    $dbname = "esp32_datalogger"; 
    $table_name = "voltage_readings";   // ⬅️ Target table for voltage data

    // ------------------------------------
    // 2. CONNECT TO DATABASE
    // ------------------------------------
    $conn = new mysqli($servername, $username, $password, $dbname);

    // Check connection
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    
    // ------------------------------------
    // 3. PROCESS POST DATA
    // ------------------------------------
    if ($_SERVER["REQUEST_METHOD"] == "POST") {

        // 1. Retrieve the voltage value sent by the ESP32 using the key 'voltage_value'
        $voltage_raw = $_POST['voltage_value']; 
        
        // 2. Normalize and Sanitize the data (to handle decimal formats safely)
        $voltage_clean = str_replace(',', '.', $voltage_raw); // Ensures dot '.' is used for decimal
        $voltage_sql = $conn->real_escape_string($voltage_clean); // Sanitize the string

        // 3. Check if data is present and numeric
        if (empty($voltage_sql) || !is_numeric($voltage_sql)) {
            echo "Error: Invalid or non-numeric voltage data received.";
            $conn->close();
            exit();
        }

        // ------------------------------------
        // 4. PREPARE and EXECUTE SQL INSERT
        // ------------------------------------
        // Insert the voltage string into the DECIMAL column
        $sql = "INSERT INTO $table_name (voltage_value) 
                VALUES ('$voltage_sql')";

        if ($conn->query($sql) === TRUE) {
            echo "New voltage record created successfully: Value=$voltage_sql V";
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