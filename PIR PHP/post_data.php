<?php
    // ------------------------------------
    // 1. DATABASE CONFIGURATION
    // ------------------------------------
    $servername = "localhost"; 
    $username = "root";       
    $password = "";           
    $dbname = "esp32_datalogger"; 
    $table_name = "motion_logs";   // ⬅️ UPDATED to the new table name

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

        // Retrieve the motion key sent by the ESP32
        $motion_status_raw = $_POST['motion']; 
        
        // Sanitize the string value for SQL insertion
        $motion_status_sql = $conn->real_escape_string($motion_status_raw);

        // Check for empty data
        if (empty($motion_status_sql)) {
            echo "Error: Motion status data is empty.";
            $conn->close();
            exit();
        }

        // ------------------------------------
        // 4. PREPARE and EXECUTE SQL INSERT
        // ------------------------------------
        // Insert only the motion status into the new table
        $sql = "INSERT INTO $table_name (motion_status) 
                VALUES ('$motion_status_sql')";

        if ($conn->query($sql) === TRUE) {
            echo "New motion record created successfully: Status=$motion_status_sql";
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