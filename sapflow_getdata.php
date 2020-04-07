<?php
//send back an array in JSON format with pairs of time and value.  
//Thinking to have return only one set of data at a time...
//Takes field, tree id
//Should I do an ajax call every time I want to get new graphs?
//	ex.  want to change time scale
//
//Start with just a single graph with sapflow values vs time


//establish connection to database
require_once '../tree_capstone_files/pdo_connect.php';
try {
	//get latest set of data
	//might keep timestamp? IDK
		//default id is 1
		$id_provided = 0;
		$rows_max= 100;
		if ($_GET["most_recent"]) {
			$dates_after= $_GET["most_recent"];
		}
		if ($_GET["id"]) {
			$id_provided = $_GET["id"];
		}
		$stmt = $pdo->prepare('SELECT Timestamp, flow, id , weight, temperature, maxtemp FROM sap WHERE id = :id order by MeasurementID desc'
    . ' limit 100');
		$stmt->bindParam(':id', $id_provided, PDO::PARAM_INT);
    $stmt->execute();
    $result = $stmt->fetchAll(PDO::FETCH_ASSOC);
    //parse into array
	    $send = "{\"time\": [";
    foreach ($result as $current_row) {
	     $send = $send . "\""; 
	     $send = $send . $current_row['Timestamp']; 
	     $send = $send . "\","; 
    }
    $send = substr($send, 0, -2);
    $send = $send . "\"],\"sapflow\": [";
    foreach ($result as $current_row) {
	     $send = $send . "\""; 
	     $send = $send . $current_row['flow']; 
	     $send = $send . "\","; 
    }
    $send = substr($send, 0, -2);
    $send = $send . "\"],\"weight\": [";
    foreach ($result as $current_row) {
	     $send = $send . "\""; 
	     $send = $send . $current_row['weight']; 
	     $send = $send . "\","; 
    }
    $send = substr($send, 0, -2);
    $send = $send . "\"],\"temperature\": [";
    foreach ($result as $current_row) {
	     $send = $send . "\""; 
	     $send = $send . $current_row['temperature']; 
	     $send = $send . "\","; 
    }
    $send = substr($send, 0, -2);
    $send = $send . "\"],\"id\": [";
    foreach ($result as $current_row) {
	     $send = $send . "\""; 
	     $send = $send . $current_row['id']; 
	     $send = $send . "\","; 
    }
    $send = substr($send, 0, -2);
    $send = $send . "\"],\"maxtemp\": [";
    foreach ($result as $current_row) {
	     $send = $send . "\""; 
	     $send = $send . $current_row['maxtemp']; 
	     $send = $send . "\","; 
    }
    $send = substr($send, 0, -2);
    $send = $send . "\"]}";
    //send back the results
    //TESTING ONLY DELETE AFTER TESTING
    //$send = $send . $dates_after;
    echo $send;
}
catch(PDOException $e) {
    echo "Error: " . $e->getMessage();
}
$conn = null;
?>
