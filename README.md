# Simple Cloud Client

A sane and simple way of connecting mbed OS 5 devices to mbed Cloud. It's designed to:

* Enable mbed Cloud Connect and mbed Cloud Update to applications in five lines of code.
* Run separate from your main application, it does not take over your main loop.
* Provide 'cloud variables'. Variables that are automatically synced through mbed Cloud Connect.
* Help users avoid doing blocking network operations in interrupt contexts, by automatically defering actions to a separate thread.

This library is a simpler interface to mbed Cloud Client, making it trivial to expose sensors, actuators and other variables to the cloud. It does not require you to change how you write your code. You can take any local variable, swap it out for a call to Simple Cloud Client, and the variable will automatically be synchronised with mbed Cloud.

[**Example program here (including instructions for firmware updates)**](https://github.com/armmbed/simple-cloud-client-example).

## Usage

1. Add this library to your project:

    ```
    $ mbed add git@github.com:ARMmbed/simple-cloud-client.git
    ```

1. Add your mbed Cloud certificate to your project (`identity_dev_security.c` file).
1. Generate an update certificate via:

    ```
    $ python3 simple-cloud-client/tools/generate-update-certs.py -v yourdomain.com -m your-device-model
    ```

1. Reference the library from your main.cpp file:

    ```cpp
    #include "simple-cloud-client.h"

    SimpleMbedClient client;
    SimpleResourceInt myResource = client.define_resource("3700/0/5501", 42);

    void on_registered() {
        printf("Registered with mbed Cloud\n");
    }

    int main() {
        NetworkInterface* network = /* get connectivity from somewhere */;

        bool setup = client.setup(network);
        if (!setup) {
            printf("Client setup failed\n");
            return 1;
        }
        client.on_registered(&registered);

        wait(osWaitForever);
    }
    ```

## Defining cloud variables

A cloud variable acts like a normal variable, but automatically syncs its value with mbed Cloud. You can create such a variable via a call to `client.define_resource()`. For example, this is how you expose a simple light sensor to the cloud.

```cpp
SimpleMbedClient client;

// This variable will be available under 3301/0/5700 in mbed Cloud. It's read-only (from the Cloud).
SimpleResourceInt light_sensor_value = client.define_resource("3301/0/5700", 0, M2MBase::GET_ALLOWED);

// Define the sensor
AnalogIn light_sensor(A0);

void read_light_sensor() {
    // Update the variable to sync to mbed Cloud
    light_sensor_value = light_sensor.read_u16();
}

int main() {
    Ticker t;
    t.attach(client.eventQueue->event(&read_light_sensor));

    // other setup code here
}
```

### SimpleResource* API

You can define a new variable by a call to `client.define_resource`. This function takes five arguments:

1.  `path` - The URL on which your variable is exposed in mbed Cloud. Needs to be three (3) segments, split by a slash (/) in the form of 'sensor/0/value'. The second segment always needs to be numeric.
2.  `defaultValue` - The default value of the variable. Needs to be either a string or an integer. Depending on the type that you pass in here the type of the variable is defined.
3.  `operation` - Some variables might be read-only or write-only (seen from the cloud). Use the operation to define these constraints. It's of type [M2MBase::Operation](https://docs.mbed.com/docs/mbed-client-guide/en/latest/api/classM2MBase.html#a8ddff21b51b1283c4f0fe463e5a2a6ee). Default is GET_PUT_ALLOWED.
4.  `observable` - If set to false, cloud applications cannot subscribe to updates on this variable. Default is true.
5.  `callback` - Function pointer which is called whenever the value of the variable is changed from the cloud.

The type returned by the function is either `SimpleResourceInt`, `SimpleResourceString` or `SimpleResourceFloat`. You can assign and read from these variables like any normal local variable.

```cpp
void name_updated(string new_value) {
    printf("Value is now %s\n", new_value.c_str());
}

SimpleResourceString name = client.define_resource("device/0/name", "jan", M2MBase::GET_PUT_ALLOWED, true, &name_updated);

// we can read and write to this variable, e.g.:
stringstream ss;
ss << name;

// or
name = "pietje";

// are all valid
```

## Defining functions

You can define functions, which do not have a value, but can just be invoked from the cloud, by a call to `client.define_function`. This function takes two arguments:

1.  `path` - The URL on which your variable is exposed in mbed Cloud. Needs to be three (3) segments, split by a slash (/) in the form of 'sensor/0/value'. The second segment always needs to be numeric.
2.  `callback` - Function pointer which is invoked when the function is called. Takes in a pointer, which contains the data being passed in from the cloud.

```cpp
void play(void* data) {
    if (data) { // data can be NULL!
        // cast it to something useful
    }
}

client.define_function("music/0/play", &play);
```

## Accessing the underlying M2MResource

If you need access to the underlying [M2MResource](https://docs.mbed.com/docs/mbed-client-guide/en/latest/api/classM2MResource.html) you can do so by calling `get_resource` on a variable, or by calling `client.get_resource` if it's a function.

```cpp
SimpleResourceInt led = client.define_resource("led/0/value", true);

client.define_function("led/0/toggle", &toggleLed);

// now to get the resource
M2MResource* ledResource = led.get_resource();
M2MResource* toggleResource = client.get_resource("led/0/toggle");
```

## Printing variables

Unfortunately `printf` is kind of dumb, and does not automatically cast the variables. If you want to print any of the Simple Cloud Client variables you'll need to cast yourself.

```cpp
SimpleResourceInt led = client.define_resource("led/0/value", true);

printf("Value is currently %d\n", static_cast<int>(led));
```

## Event Queue

Simple Cloud Client uses an [mbed-events EventQueue](https://github.com/ARMmbed/mbed-os/tree/master/events) - running on a separate RTOS thread - to handle incoming events without blocking the main loop. Both the thread and event queue are created when initializing the library. You can override this behavior by providing your own event queue. In this case no thread is created.

```cpp
EventQueue myQueue;
SimpleMbedClient client(&myQueue);
```

You can also use the queue to process your own events, which is very useful when dealing with ISRs. The queue is accessible through the `eventQueue()` function on the client object and returns a pointer to the queue.

```cpp
SimpleMbedClient client;

InterruptIn btn(D2);

int main() {
  btn.fall(client.eventQueue()->event(&fall));
}
```
