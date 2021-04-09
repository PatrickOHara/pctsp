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
You can build the docker file as follows:

```bash
docker build -t template:latest .
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

We are moving towards adding [type hints](https://docs.python.org/3.7/library/typing.html) so you may optionally add types to your code.
In which case you do not need to include types in your google style docstrings.


## Gitmoji
We like [gitmoji](https://gitmoji.carloscuesta.me/) for an emoji guide to our commit messages.
You might consider (entirely optional) using the [gitmoji-cli](https://github.com/carloscuesta/gitmoji-cli) as a hook when writing commit messages.

## Working on an issue
The general workflow for contributing to the project is to first choose and issue (or create one) to work on and assign yourself to the issues.

You are encouraged to open a pull request earlier rather than later (either a `draft pull request` or add `WIP` to the title) so others know what you are working on.

How you label branches is optional, but we encourage using `<issue-number>_<description_of_issue>` where `<issue-number>` is the github issue number and `<description_of_issue>` is a very short description of the issue. For example `22_improve_docs`.
