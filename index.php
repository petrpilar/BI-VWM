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
                <h2>Extended boolean model</h2>
                <form method="get" action="src/find.php">
                    <ul class="form-style-1">
                        <li>
                            <label>Query <span class="required">*</span></label>
                            <input type="text" name="query" class="field-long" />
                        </li>
                        <li>
                            <label>Number of results <span class="required">*</span></label>
                            <input type="number" name="number" class="field-long" />
                        </li>
                        <li>
                            <input type="submit" value="Find" />
                        </li>
                    </ul>
                </form>
            </div>
        </section>
    </body>
</html>