#README
# This script tweets the trees that failed between 1 and 2 hours previous. This range allows the user to modifiy
# Smartprobe transmission period without having to worry about false negatives.
# The number of trees that updates are posted for comes from the max tree id from the SQL database.

<?php
//open connection
require_once './pdo_connect.php';

//get number of trees in database using MAX
$stmt = $pdo->prepare('SELECT MAX(id) FROM sap');
$stmt->execute();
$total_trees =$stmt->fetchColumn();
echo($total_trees);
echo("\n");

//TEST ONLY
$total_trees = 1;

for ($tree = 1; $tree <= $total_trees; $tree++) {
	//Figure out the when this tree's last log was submitted
	$stmt = $pdo->prepare('SELECT Timestamp FROM sap order by MeasurementID desc');
	$stmt->execute();
	$last_logged = $stmt->fetchColumn();
	$dt_last_logged = strtotime($last_logged);
	echo($last_logged);
	echo("\n");

        //Find an hour before current time
	$now = new DateTime();
	//$now->sub(new DateInterval('PT1H'));
	$str_lasthr = date_format($now, 'Y-m-d H:i:s');	
	//Find two hours before curren time
	$now->sub(new DateInterval('PT1H'));
	$second_lasthr = date_format($now, 'Y-m-d H:i:s');

	echo($str_lasthr);
	echo("\n");
	echo($second_lasthr);

	//If a tree hasn't sent logs for the past hour, tweet that an error occured
	if (($second_lasthr < $dt_last_logged) && ($dt_last_logged < $str_lasthr)){
		$stmt = $pdo->prepare('SELECT AVG(flow) FROM sap limit 20');
		$stmt->execute();
		$average_sapflow = $stmt->fetchColumn();
		$data = $average_sapflow;
		$date = date("F j, Y", time() - 60 * 60 * 24);
		$id = $tree;
		$message = "Tree " . $id . " stopped logging at" . $dt_last_logged;
		$call = sprintf("twurl -d 'status=%s' /1.1/statuses/update.json",$message);
		//echo exec($call);
		echo($message);
	}
}
?>
