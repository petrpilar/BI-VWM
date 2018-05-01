<?php 
	if (isset($_GET["query"]) && isset($_GET["n_of_res"])){
		Exec("./src/test.exe", $out);
	}
