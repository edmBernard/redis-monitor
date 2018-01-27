<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="utf-8">
    <link rel="icon" type="image/png" href="static/line-chart.png"/>
    <title>Redis Monitoring</title>

    <link rel="stylesheet" href="static/bulma.min.css">
    <link rel="stylesheet" href="static/bulma.min.css.map">
    <link rel="stylesheet" href="static/billboard.min.css">
    <script src="static/d3.min.js" charset="utf-8"></script>
    <script src="static/billboard.min.js" charset="utf-8"></script>
</head>

<body>
    <section class="section">
        <div class="container">
            <div class="columns is-desktop">
{% for key in keys %}
                <div class="column ">
                    <div id="{{key}}"></div>
                </div>
{% endfor %}
</div>          
</div>
</section>
<script>
{% for key in keys %}
    var chart = bb.generate({
        data: {
            x: "x",
            columns: [
            ["x", "2013-01-01", "2013-01-02", "2013-01-03", "2013-01-04", "2013-01-05", "2013-01-06"],
            ["data1", 30, 200, 100, 400, 150, 250],
            ["data2", 130, 340, 200, 500, 250, 350]
            ]
        },
        axis: {
            x: {
                type: "timeseries",
                tick: {
                    format: "%Y-%m-%d"
                }
            }
        },
        bindto: "#{{key}}"
    });
{% endfor %}

    </script>
</body>

</html>