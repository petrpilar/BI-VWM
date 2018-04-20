<?php

//files count
//https://stackoverflow.com/questions/12801370/count-how-many-files-in-directory-php
$fi = new FilesystemIterator(__DIR__, FilesystemIterator::SKIP_DOTS);
$fileCount = iterator_count($fi);
//printf("There were %d Files", iterator_count($fi));
 
for($i = 0; $i < $fileCount; $i++) {
	$file = fopen("doc" + $i + ".txt", "r") or die("Unable to open file doc" + $i + ".txt.");
	//https://stackoverflow.com/questions/17206468/php-reading-word-by-word
	while(($line = fgets($handle)) !== false) {
		$word_arr = explode(" ", $line); //return word array
        foreach($word_arr as $word) {
            //podivej se do tabulky (sloupce s lemmaty) a zjisti, zda tam je, pokud ano, jen pridej u dokumentu 1, jinak pridej slovo a vsude krome aktualniho dokumentu napis 0
			echo $word; // required output
        }
	}
	fclose($file);
}


?>