import requests
import json


def pretty_print_request(request):
    print('\n{}\n{}\n\n{}\n\n{}\n'.format(
        '-----------Request----------->',
        request.method + ' ' + request.url,
        '\n'.join('{}: {}'.format(k, v) for k, v in request.headers.items()),
        request.body)
    )


def pretty_print_response(response):
    print('\n{}\n{}\n\n{}\n\n{}\n'.format(
        '<-----------Response-----------',
        'Status code:' + str(response.status_code),
        '\n'.join('{}: {}'.format(k, v) for k, v in response.headers.items()),
        response.text)
    )


HOST = '127.0.0.1:20322'


def log_request_response(http_verb):
    def wrapped(*args, **kwargs):
        resp = http_verb(*args, **kwargs)
        pretty_print_request(resp.request)
        pretty_print_response(resp)
        return resp

    return wrapped


@log_request_response
def send_post(url, headers, payload):
    return requests.post(url, headers=headers, data=json.dumps(payload, indent=4))


@log_request_response
def send_get(url):
    return requests.get(url)


def test_cinemas_post():
    url = f"http://{HOST}/cinemas/"

    headers = {'Content-Type': 'application/json'}

    payload = {"cinemas": [
        {"name": "PiterLand",
         "width": 2,
         "height": 3,
         "films": ["Once upon in hollywood", "Survived"]},
        {"name": "Galary",
         "films": ["Survived", "Ford VS Ferrari"],
         "width": 3,
         "height": 4
         }
    ]
    }

    resp = send_post(url, headers, payload)
    assert resp.status_code == 201


def test_cinemas_post_failed():
    url = f"http://{HOST}/cinemas/"

    headers = {'Content-Type': 'application/json'}

    payload = {"cinemas": [
        {"name": "PiterLand",
         "width": 10,
         "height": 20,
         "films": ["Once upon in hollywood", "Survived"]},
        {"name": "Galary",
         "films": ["Survived", "Ford VS Ferrari"],
         "width": 10,
         "height": 5
         }
    ]
    }

    resp = send_post(url, headers, payload)
    assert resp.status_code == 400
    assert resp.headers['Content-type'] == "application/json"
    resp_body = resp.json()
    assert 'reason' in resp_body
    assert resp_body['reason'] == 'Failed to add cinema'


def test_cinemas_get():
    url = f"http://{HOST}/cinemas/"

    resp = send_get(url)
    assert resp.status_code == 200
    assert resp.headers['Content-type'] == "application/json"
    resp_body = resp.json()
    assert 'cinemas' in resp_body
    assert sorted(resp_body['cinemas']) == sorted(['PiterLand', 'Galary'])

def test_cinema_films():
    url = f"http://{HOST}/cinemas/Galary"

    resp = send_get(url)
    assert resp.status_code == 200
    assert resp.headers['Content-type'] == "application/json"
    resp_body = resp.json()
    assert 'films' in resp_body
    assert sorted(resp_body['films']) == sorted(['Survived', 'Ford VS Ferrari'])


def test_cinemas_films():
    url = f"http://{HOST}/cinemas/films"

    resp = send_get(url)
    assert resp.status_code == 200
    assert resp.headers['Content-type'] == "application/json"
    resp_body = resp.json()
    assert 'films' in resp_body
    assert sorted(resp_body['films']) == sorted(["Once upon in hollywood", "Survived", "Ford VS Ferrari"])


def test_cinemas_for_film():
    url = f"http://{HOST}/cinemas/films/Survived"

    resp = send_get(url)
    assert resp.status_code == 200
    assert resp.headers['Content-type'] == "application/json"
    resp_body = resp.json()
    assert 'cinemas' in resp_body
    assert sorted(resp_body['cinemas']) == sorted(["PiterLand", "Galary"])


def test_seats_for_film():
    url = f"http://{HOST}/cinemas/PiterLand/Survived"

    resp = send_get(url)
    assert resp.status_code == 200
    assert resp.headers['Content-type'] == "application/json"
    resp_body = resp.json()
    assert 'seats' in resp_body
    assert sorted(resp_body['seats']) == sorted(
        ["0row0seat", "0row1seat", "0row2seat", "1row0seat", "1row1seat", "1row2seat"])


def test_book_seats_for_film_first_try():
    url = f"http://{HOST}/cinemas/PiterLand/Survived"

    headers = {'Content-Type': 'application/json'}

    payload = {"seats": ['0row0seat', '0row1seat']}

    resp = send_post(url, headers, payload)
    assert resp.status_code == 201


def test_book_seats_for_film_second_try():
    url = f"http://{HOST}/cinemas/PiterLand/Survived"

    headers = {'Content-Type': 'application/json'}

    payload = {"seats": ['0row2seat', '1row1seat']}

    resp = send_post(url, headers, payload)
    assert resp.status_code == 201


def test_book_seats_for_film_final_try():
    url = f"http://{HOST}/cinemas/PiterLand/Survived"

    headers = {'Content-Type': 'application/json'}

    payload = {"seats": ['0row0seat', '1row2seat']}

    resp = send_post(url, headers, payload)
    assert resp.status_code == 400
    assert resp.headers['Content-type'] == "application/json"
    resp_body = resp.json()
    assert 'busy_seats' in resp_body
    assert sorted(resp_body['busy_seats']) == sorted(["0row0seat"])