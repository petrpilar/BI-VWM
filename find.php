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
                    $input = $_GET["number"] . ' "' . $_GET["query"] . '"';

                    if (file_exists($file)) {
                        echo "<p>file " . $file . " exists</p>";
                    } else {
                        echo "<p>file " . $file . " does not exists</p>";
                    }

                    $return = 5;
                    $toexec = $file . " " . $input;

                    exec($toexec, $lines, $return);
                    //                    $lines = system($toexec, $return);
                    var_dump($toexec);
                    echo "<br>";
                    var_dump($lines);
                    echo "<br>";
                    var_dump($return);
                    echo "<br>";

                    foreach ($lines as $item) {
                        echo '<a href="./data/doc' . $item . '.txt">' . 'doc' . $item . '.txt' . "</a><br>";
                        echo "<br>";
                    }
                } else {
                    echo "<p>its working but sadly not now (=_=)</p>";
                }
                ?>
            </div>
        </section>
    </body>
</html>
