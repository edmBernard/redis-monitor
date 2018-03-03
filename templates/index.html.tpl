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
    <script src="static/visibility.min.js" charset="utf-8"></script>
</head>

<body>
    <section class="section">
        <div class="container">
            <h1 class="title">Keys length monitoring</h1>
            <div id="keysLength"></div>
        </div>
    </section>
    <section class="section">
        <div class="container">
            <h1 class="title">PubSub speed monitoring</h1>
            <div id="patternsSpeed"></div>
        </div>
    </section>
<script>

    var chart_keys = bb.generate({
        data: {
            xs: {
                {% for i in keys %}
                {{i}}: "x{{i}}",
                {% endfor %}
                padding: "padding"
            },
            xFormat: '%Y-%m-%d %H:%M:%S',
            columns: [
            ]
        },
        axis: {
            x: {
                label: "Time",
                type: "timeseries",
                tick: {
                    fit: false,
                    format: "%Y-%m-%d %H:%M:%S"
                }
            },
            y: {
                label: "Keys length",
                min: 0
            }
        },
        subchart: {
            show: true
        },
        title: {
            text: 'Keys length monitoring'
        },
        grid: {
            x: {
              show: true
            },
            y: {
              show: true
            }
        },
        bindto: "#keysLength"
    });

    var chart_patterns = bb.generate({
        data: {
            xs: {
                {% for i in patterns %}
                "{{i}}": "x{{i}}",
                {% endfor %}
                padding: "padding"
            },
            xFormat: '%Y-%m-%d %H:%M:%S',
            columns: [
            ]
        },
        axis: {
            x: {
                label: "Time",
                type: "timeseries",
                tick: {
                    fit: false,
                    format: "%Y-%m-%d %H:%M:%S"
                }
            },
            y: {
                label: "Publish speed",
                min: 0
            }
        },
        subchart: {
            show: true
        },
        title: {
            text: 'Publish speed monitoring'
        },
        grid: {
            x: {
              show: true
            },
            y: {
              show: true
            }
        },
        bindto: "#patternsSpeed"
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

    httpGetAsync('/updatekeys', function(response) {
        response = JSON.parse(response);
        for (let i = 0; i < response.length; i++) {
            chart_keys.load({
                columns: [
                    ["x" + response[i].id].concat(response[i].abscisse),
                    [response[i].id].concat(response[i].ordinate)
                ]
            });
        }
    });

    httpGetAsync('/updatepatterns', function(response) {
        response = JSON.parse(response);
        for (let i = 0; i < response.length; i++) {
            chart_patterns.load({
                columns: [
                    ["x" + response[i].id].concat(response[i].abscisse),
                    [response[i].id].concat(response[i].ordinate)
                ]
            });
        }
    });

    ws = new WebSocket("ws://" + window.location.hostname + ":" + window.location.port);
	ws.onmessage = function(e) {
        if (!Visibility.hidden()) {
            let data = JSON.parse(e.data);

            let columns_to_add = [];
            for (let i = 0; i < data.keys.length; i++) {
                columns_to_add.push(["x" + data.keys[i].id].concat(data.date))
                columns_to_add.push([data.keys[i].id].concat(data.keys[i].value))
            }

            chart_keys.flow({
                columns: columns_to_add,
                length: 0,
                duration: 0
            });

            columns_to_add = [];
            for (let i = 0; i < data.patterns.length; i++) {
                columns_to_add.push(["x" + data.patterns[i].id].concat(data.date))
                columns_to_add.push([data.patterns[i].id].concat(data.patterns[i].value))
            }

            chart_patterns.flow({
                columns: columns_to_add,
                length: 0,
                duration: 0
            });
        };
	};
	ws.onclose = function() {
		console.log("disconnected from server");
	};

    var listener = Visibility.change(function (e, state) {
        if (!Visibility.hidden()) {
            window.location.reload();
        }
    });

    </script>
</body>

</html>