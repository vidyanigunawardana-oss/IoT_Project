<?php
    // ------------------------------------
    // 1. DATABASE CONFIGURATION
    // ------------------------------------
    $servername = "localhost"; // XAMPP MySQL server
    $username = "root";        // Default XAMPP MySQL username
    $password = "";            // Default XAMPP MySQL password (usually blank)
    $dbname = "esp32_datalogger"; 
    $table_name = "current_readings";   // ⬅️ Target table for current sensor data

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

        // 1. Retrieve the amperage value sent by the ESP32 using the key 'amperage'
        $amperage_raw = $_POST['amperage']; 
        
        // 2. Normalize and Sanitize the data (to handle decimal formats safely)
        $amperage_clean = str_replace(',', '.', $amperage_raw); // Replace comma with dot if locale is an issue
        $amperage_sql = $conn->real_escape_string($amperage_clean); // Sanitize the string

        // 3. Check if data is present and numeric
        if (empty($amperage_sql) || !is_numeric($amperage_sql)) {
            echo "Error: Invalid or non-numeric amperage data received.";
            $conn->close();
            exit();
        }

        // ------------------------------------
        // 4. PREPARE and EXECUTE SQL INSERT
        // ------------------------------------
        // Insert the amperage string into the DECIMAL column
        $sql = "INSERT INTO $table_name (amperage) 
                VALUES ('$amperage_sql')";

        if ($conn->query($sql) === TRUE) {
            echo "New current record created successfully: Amperage=$amperage_sql A";
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