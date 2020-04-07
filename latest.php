
<table style='border: solid 1px black;'>
<tr><th>Sample</th><th>System Time</th><th>Sapflow</th><th>Weight</th><th>Temperature</th><th>Embedded Time</th><th>Tree ID</th><th>Max Heater Temp</th></tr>
<?php
class TableRows extends RecursiveIteratorIterator {
    function __construct($it) {
        parent::__construct($it, self::LEAVES_ONLY);
    }

    function current() {
        return "<td style='width:150px;border:1px solid black;'>" .
parent::current(). "</td>";
    }

    function beginChildren() {
        echo "<tr>";
    }

    function endChildren() {
        echo "</tr>" . "\n";
    }
}

	//include 'pdo_connect.php';
require_once '../tree_capstone_files/pdo_connect.php';

try {
		$stmt = $pdo->prepare('SELECT * FROM sap order by MeasurementID desc'
    . ' limit 100');
    $stmt->execute();

    // set the resulting array to associative
    $result = $stmt->setFetchMode(PDO::FETCH_ASSOC);
    foreach(new TableRows(new
RecursiveArrayIterator($stmt->fetchAll())) as $k=>$v) {
        echo $v;
    }
}
catch(PDOException $e) {
    echo "Error: " . $e->getMessage();
}
$conn = null;
echo "</table>";
?> 
