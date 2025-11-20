<?php
    // ------------------------------------
    // 1. DATABASE CONFIGURATION
    // ------------------------------------
    $servername = "localhost"; // XAMPP MySQL server
    $username = "root";        // Default XAMPP MySQL username
    $password = "";            // Default XAMPP MySQL password (usually blank)
    $dbname = "esp32_datalogger"; 
    $table_name = "relay_logs";   // ⬅️ Target table for relay data

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

        // 1. Retrieve the relay state sent by the ESP32 using the key 'state'
        $relay_state_raw = $_POST['state']; 
        
        // 2. Sanitize the string value for SQL insertion (protects against injection)
        $relay_state_sql = $conn->real_escape_string($relay_state_raw);

        // 3. Check for valid data
        if (empty($relay_state_sql) || 
            ($relay_state_sql != "ON" && $relay_state_sql != "OFF")) {
            
            echo "Error: Invalid or empty relay state received.";
            $conn->close();
            exit();
        }

        // ------------------------------------
        // 4. PREPARE and EXECUTE SQL INSERT
        // ------------------------------------
        // Insert the relay state string into the relay_state column
        $sql = "INSERT INTO $table_name (relay_state) 
                VALUES ('$relay_state_sql')";

        if ($conn->query($sql) === TRUE) {
            echo "New relay record created successfully: State=$relay_state_sql";
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