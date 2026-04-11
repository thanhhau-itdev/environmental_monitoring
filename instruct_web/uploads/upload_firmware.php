<?php

$conn = new mysqli("localhost", "root", "", "htqt");
$firmware = $_POST("firmware");
$sql = "SELECT status FROM upload_firmwares WHERE firmware = '$firmware'";

if ($conn->query($sql)->fetch_assoc()['status'] == 1) {
    echo json_encode([
        'file' => "http://192.168.1.3/htqt/file/main3.ino.bin",
	'status' => $firmware
    ]);
} else {
    echo json_encode([
        'error' => "Update failed !"
    ]);
}

mysqli_close($conn);