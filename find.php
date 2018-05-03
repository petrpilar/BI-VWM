<?php

if (isset($_GET["query"]) && isset($_GET["number"])) {
    $file = 'src/test.exe';
    if (file_exists($file)) {
        echo "ANO ";
    } else {
        echo "NE ";
    }
    exec($file, $out, $return);
    var_dump($out);
    var_dump($return);

    foreach ($out as $item) {
        echo "<h1>" . $item . "</h1>";
    }
}
