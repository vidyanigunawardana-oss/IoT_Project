<?php
    // ------------------------------------
    // 1. DATABASE CONFIGURATION
    // ------------------------------------
    $servername = "localhost";
    $username = "root";       
    $password = "";           
    $dbname = "esp32_datalogger"; 
    $table_name = "dht_readings"; 

    // ------------------------------------
    // 2. CONNECT TO DATABASE
    // ------------------------------------
    $conn = new mysqli($servername, $username, $password, $dbname);

    // Check connection
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    
    // ------------------------------------
    // 3. PROCESS POST DATA (CLEANED)
    // ------------------------------------
    if ($_SERVER["REQUEST_METHOD"] == "POST") {

        // 1. Retrieve the POST variables as strings
        $temp_str = $_POST['temperature'];
        $hum_str = $_POST['humidity'];
        
        // 2. FIX: Force the decimal separator to a dot (for database compatibility)
        // This is done on the retrieved string variables ($temp_str, $hum_str)
        $temp_clean = str_replace(',', '.', $temp_str);
        $hum_clean = str_replace(',', '.', $hum_str);
            
        // 3. Sanitize the clean string values for SQL insertion
        // We rely on MySQL to convert the valid string ("25.40") into a DECIMAL type
        $temperature_sql = $conn->real_escape_string($temp_clean);
        $humidity_sql = $conn->real_escape_string($hum_clean);

        // Check if data is present and numeric (using the final SQL-safe variables)
        if (empty($temperature_sql) || empty($humidity_sql) || !is_numeric($temperature_sql) || !is_numeric($humidity_sql)) {
            echo "Error: Invalid or non-numeric data received after cleaning.";
            $conn->close();
            exit();
        }

        // ------------------------------------
        // 4. PREPARE and EXECUTE SQL INSERT
        // ------------------------------------
        $sql = "INSERT INTO $table_name (temperature, humidity) 
                VALUES ('$temperature_sql', '$humidity_sql')"; // USE THE CLEANED/SANITIZED VARIABLES

        if ($conn->query($sql) === TRUE) {
            echo "New record created successfully: Temp=$temperature_sql, Hum=$humidity_sql";
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
