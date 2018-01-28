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
                    <div id="keysLength"></div>
                </div>
            </div>          
        </div>
    </section>
<script>

    var chart = bb.generate({
        data: {
            x: "x",
            xFormat: '%Y-%m-%d %H:%M:%S',
            columns: [
            ]
        },
        axis: {
            x: {
                label: "Time",
                type: "timeseries",
                tick: {
                    rotate: 45,
                    fit: false,
                    format: "%Y-%m-%d %H:%M:%S"
                }
            },
            y: {
                label: "keys' length"
            }
        },
        title: {
            text: 'Keys length monitoring'
        },
        grid: {
            y: {
              show: true
            }
        },
        bindto: "#keysLength"
    });

    function httpGetAsync(theUrl, callback)
    {
        var xmlHttp = new XMLHttpRequest();
        xmlHttp.onreadystatechange = function() { 
            if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
                callback(xmlHttp.responseText);
        }
        xmlHttp.open("GET", theUrl, true); // true for asynchronous 
        xmlHttp.send(null);
    }

    httpGetAsync('/update', function(response) {
        response = JSON.parse(response);
        for (let i = 0; i < response.length; i++) {
            chart.load({
                columns: [
                    ["x"].concat(response[i].abscisse),
                    [response[i].id].concat(response[i].ordinate)
                ]
            });
        }
    });



    </script>
</body>

</html>