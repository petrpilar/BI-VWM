<!DOCTYPE html>
<html>
    <head>
        <title>Extended boolean model</title>
        <link rel="stylesheet" type="text/css" href="style.css">
    </head>
    <body>
        <header>
            <nav>
                <div class="main-wrapper">
                    <ul>
                        <li><a href="index.php">Home</a></li>
                    </ul>
                </div>
            </nav>
        </header>       
        <section class="main-container">
            <div class="main-wrapper">
                <h1>Results</h1>
                <?php
                if (isset($_GET["query"]) && isset($_GET["number"])) {
                    $file = 'src\\infix_to_postfix_and_evaluate.exe';
                    $input = $_GET["number"] . " " . $_GET["query"];

                    if (file_exists($file)) {
                        echo "<p>file " . $file . " exists</p>";
                    } else {
                        echo "<p>file " . $file . " does not exists</p>";
                    }

                    $return = -1;
                    $toexec = $file . " " . $input;
                    exec($toexec, $out, $return);

                    var_dump($out);
                    var_dump($return);

                    foreach ($out as $item) {
                        echo '<a href="./data/doc' . $item . '.txt">' . 'doc' . $item . '.txt' . "</a><br>";
                    }
                } else {
                    echo "<p>its working but sadly not now (=_=)</p>";
                }
                ?>
            </div>
        </section>
    </body>
</html>
