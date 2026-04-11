<?php
$conn = new mysqli("localhost", "root", "", "htqt");

$temp = $_POST['temp'];
$ph   = $_POST['ph'];
$ec   = $_POST['ec'];
$do  = $_POST['do'];

$sql = "INSERT INTO sensors (temp, ph, ec, do)
        VALUES ('$temp', '$ph', '$ec', '$do')";

$conn->query($sql);
mysqli_close($conn);
