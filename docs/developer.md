# Developer guide

Instructions for good practises when developing python code.
Make sure you have installed the developer requirements:

```bash
pip install -r requirements.txt
```

## Running tests

We use pytest for unit testing.
Simply use the pytest command:

```bash
pytest
```

## Writing Documentation

Before being accepted into master all code should have well writen documentation.
Please use [Google Style Python Docstrings](https://sphinxcontrib-napoleon.readthedocs.io/en/latest/example_google.html).

We are using [mkdocs](https://www.mkdocs.org/#getting-started).
The docs can be generated locally as follows:

```bash
mkdocs serve
```

Adding and updating existing documentation is highly encouraged.

## Docker

[Docker is great for reprodicible research](https://reproducible-analysis-workshop.readthedocs.io/en/latest/8.Intro-Docker.html).
You can pull the latest docker image to run our code:

```bash
docker pull patrickohara/pctsp:latest
```

Alternatively you can build the docker file as follows:

```bash
docker build -t patrickohara/pctsp:latest .
```

To re-build the base SCIP image:

```bash
docker build -t patrickohara/scip:latest -f scip.dockerfile .
```

## Linting

Keeps the code neat and tidy.
We use [pylint](https://www.pylint.org/) and [mypy](http://mypy-lang.org/).

You can change pylint settings in the `.pylintrc` file.

Some of the smaller linting issues can be cured by running the [black formatter](https://github.com/psf/black):

```bash
black */
```

### Type hinting

We are moving towards adding [type hints](https://docs.python.org/3.12/library/typing.html) so you may optionally add types to your code.
In which case you do not need to include types in your google style docstrings.

