<?php

$conn = new mysqli("localhost", "root", "", "htqt");

$fw1 = $_POST["firmware_board_read_sensor"];
$fw2 = $_POST["firmware_board_upload_data"];

$sql = "SELECT firmware, name, status FROM upload_firmwares WHERE firmware IN ('$fw1','$fw2')";
$result = $conn->query($sql);
$response = [];

while ($row = $result->fetch_assoc()) {
    if ($row['status'] == 1) {

        if ($row['firmware'] == $fw1) {
            $file = "http://192.168.1.111/htqt/files/read_sensors.ino.bin";
        } else {
            $file = "http://192.168.1.111/htqt/files/upload_datas.ino.bin";
        }

        $response[] = [
            'firmware' => $row['firmware'],
            'name' => $row['name'],
            'file' => $file
        ];

        $conn->query("UPDATE upload_firmwares SET status = 0 WHERE firmware = '".$row['firmware']."'");
    }
}

if (empty($response)) {
    echo json_encode([
        "status" => "No_update !"
    ]);
} else {
    echo json_encode($response);
}

$conn->close();