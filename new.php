<?php

//include 'pdo_connect.php';
require_once '../tree_capstone_files/pdo_connect.php';
	echo "start";
	if ($_GET["flow"]) {
		$stmt = $pdo->prepare('INSERT INTO sap (Timestamp, flow, weight, temperature, time, id, maxtemp) VALUES (now(), ?, ?, ?, ?, ?, ?)');
		$stmt->execute([$_GET["flow"],$_GET["weight"],$_GET["temp"],$_GET["time"],$_GET["id"],$_GET["maxtemp"]]);
		echo "OK";
	} else {
		//primitive error logging
		$stmt = $pdo->prepare('INSERT INTO sap (Timestamp, flow, weight, temperature, time, id) VALUES (now(), ?, ?, ?, ?, 0)');
		$stmt->execute([$_GET["flow"],$_GET["weight"],$_GET["temp"],$_GET["id"]]);
		echo "NACK";

	}
?>
