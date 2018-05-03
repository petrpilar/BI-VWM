<?php

if (isset($_GET["query"]) && isset($_GET["number"])) {
    Exec("./src/test.cpp", $out);
    print $out;
}
