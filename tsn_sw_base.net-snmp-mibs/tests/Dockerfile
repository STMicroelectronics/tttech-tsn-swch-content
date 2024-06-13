FROM python:3.7.4-slim-buster

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt
RUN mkdir /code
WORKDIR /code
COPY . /code/

VOLUME /code/input
VOLUME /code/results

CMD ["/code/test.sh"]
