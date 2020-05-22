#README
# This script is paired with a crontab to tweet tree sapflow averages once a day. The number of trees that updates are posted for comes from the max
# tree id from the SQL database. Will only pull records for a tree if the database contains entries from that ID over the previous 24 hr.  
# This prevents spamming from inactive/uninstalled trees with listings in the database.

#Need to figure out which trees should be updated daily.
#	Trees that posted during most recent day?  Defined in the code?  Probabily command line arguments, made by crontab + setup/menu script
#       Currently needs previous data to be cleared from database before use

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
//$total_trees = 1;

for ($tree = 1; $tree <= $total_trees; $tree++) {
	//Figure out the when this tree's last log was submitted
	$stmt = $pdo->prepare('SELECT Timestamp FROM sap order by MeasurementID desc');
	$stmt->execute();
	$last_logged = $stmt->fetchColumn();
	$dt_last_logged = strtotime($last_logged);
	echo($last_logged);
	echo("\n");

        //Find the current time
	$now = new DateTime();
	$now->sub(new DateInterval('P1D'));
	$str_yesterday= date_format($now, 'Y-m-d H:i:s');	
	echo($str_yesterday);
	echo("\n");
        
        //only tweet update if tree had update in the previous 24hr
	if ($dt_last_logged > $str_yesterday){
		$stmt = $pdo->prepare('SELECT AVG(flow) FROM sap limit 480');
		$stmt->execute();
		$average_sapflow = $stmt->fetchColumn();
		$data = $average_sapflow;
		$date = date("F j, Y", time() - 60 * 60 * 24);
		$id = $tree;
		$message = "Tree " . $id . " had an average of " . $data . " sapflow yesterday (" . $date . ").";
		$call = sprintf("twurl -d 'status=%s' /1.1/statuses/update.json",$message);
		//actually tweets message
		//echo exec($call);
		//just print the message for testing
		echo($message);
	}
}
?>
