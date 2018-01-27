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
                <div class="column ">
                    <div id="TimeseriesChart1"></div>
                </div>
                <div class="column">
                    <div id="TimeseriesChart2"></div>
                </div>
            </div>          
        </div>
    </section>
    <script>
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
            bindto: "#TimeseriesChart1"
        });
        
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
            bindto: "#TimeseriesChart2"
        });
    </script>
</body>

</html>