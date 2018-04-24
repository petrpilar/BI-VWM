<?php include_once 'header.php' ?>

<section class="main-container">
    <div class="main-wrapper">
        <h2>Extended boolean model</h2>
        <form method="get" action="/find.php">
            <ul class="form-style-1">
                <li>
                    <label>Query <span class="required">*</span></label>
                    <input type="text" name="query" class="field-long" />
                </li>
                <li>
                    <label>Number of results <span class="required">*</span></label>
                    <input type="number" name="n_of_res" class="field-long" />
                </li>
                <li>
                    <input type="submit" value="Find" />
                </li>
            </ul>
        </form>
        <h2>Results</h2>
        <div id="results" >
            <dl>
                <dt>Black hot drink Black hot drink</dt>
                <dd>Black hot drink</dd>

                <dt>Milk</dt>
                <dd>White cold drink</dd>

                <dt>Milk</dt>
                <dd>White cold drink</dd>

                <dt>Milk</dt>
                <dd>White cold drink</dd>

                <dt>Milk</dt>
                <dd>White cold drink</dd>
            </dl>
        </div>
    </div>
</section>

<?php include_once 'footer.php' ?>
