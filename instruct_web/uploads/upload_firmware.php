<?php

$conn = new mysqli("localhost", "root", "", "htqt");

$fw1 = $_POST["firmware_board_read_sensor"];
$fw2 = $_POST["firmware_board_upload_data"];

$sql_1 = "SELECT name, status FROM upload_firmwares WHERE firmware = '$fw1'";
$sql_2 = "SELECT name, status FROM upload_firmwares WHERE firmware = '$fw2'";

$result_1 = $conn->query($sql_1)->fetch_assoc();
$result_2 = $conn->query($sql_2)->fetch_assoc();

$response = [];

if ($result_1 && $result_1['status'] == 1) {

    $response[] = [
        'firmware' => $fw1,
        'name' => $result_1['name'],
        'file' => "http://192.168.1.111/htqt/files/" . $result_1['name']
    ];

    $conn->query("UPDATE upload_firmwares SET status = 0 WHERE firmware = '$fw1'");
}

if ($result_2 && $result_2['status'] == 1) {

    $response[] = [
        'firmware' => $fw2,
        'name' => $result_2['name'],
        'file' => "http://192.168.1.111/htqt/files/" . $result_2['name']
    ];

    $conn->query("UPDATE upload_firmwares SET status = 0 WHERE firmware = '$fw2'");
}

if (empty($response)) {
    echo json_encode([
        "status" => "No_update !"
    ]);
} else {
    echo json_encode($response);
}

$conn->close();