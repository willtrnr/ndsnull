#ifndef PAYLOAD_H
#define PAYLOAD_H

#define INDEX "<!DOCTYPE html>\
<html>\
<head>\
  <title>Coveo Tester</title>\
</head>\
<body>\
  <form action=\"/\" method=\"post\">\
    <div>\
      <label for=\"firstNumber\">First Number:</label>\
      <input id=\"firstNumber\" type=\"text\" name=\"firstNumber\">\
    </div>\
    <div>\
      <label for=\"secondNumber\">Second Number:</label>\
      <input id=\"secondNumber\" type=\"text\" name=\"secondNumber\">\
    </div>\
    <button type=\"submit\">Test</button>\
  </form>\
</body>\
</html>"

#define PAYLOAD "%d"

#endif
