#Tweets menu should allow 
#	1. Setting when/if tweeting occurs
#	2. Which trees will tweet
#	3. If only alerts should tweet


<?php
$data = 1.1;
$date = date("F j, Y", time() - 60 * 60 * 24);
$id = 0;
$message = "Tree " . $id . " recieved " . $data . " in of rainfall yesterday (" . $date . ").";
$call = sprintf("twurl -d 'status=%s' /1.1/statuses/update.json",$message);
echo exec($call);
?>
