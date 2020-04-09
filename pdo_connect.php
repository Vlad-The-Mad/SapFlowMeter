<?php
$dsn = "mysql:host=localhost;dbname=sapflow_pi_mysql;charset=utf8mb4"
$options = [
	PDO::ATTR_EMULATE_PREPARES => false,
	PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
	PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
];
$username = 'veselyv';
$password = 'Amigos';
try {
	$pdo = new PDO($dsn, $username, $password, $options);
} catch (Exception $e) {
	error_log($e->getMessage());
	exit('comething weird happened');
}
?>
