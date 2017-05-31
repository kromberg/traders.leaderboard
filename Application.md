# Application and Traffic Generator
Almost all steps are similar
## Initialization
Logger must be configured and started before the application [Logger information](Logger.md)
```c++
// Application
app::Application& application = app::Application::instance();
Result res = application.initialize();
if (Result::SUCCESS != res)
{
    // handle error
}
// Traffic generator
tg::Generator& gen = tg::Generator::instance();
Result res = gen.initialize();
if (Result::SUCCESS != res)
{
    // handle error
}
```
## Configuration.
For more informartion, please, see [Configuration information](Config.md)
```c++
// Application
Result res = application.configure(cfgName);
if (Result::SUCCESS != res)
{
    // handle error
}
// Traffic generator
Result res = gen.configure(cfgName);
if (Result::SUCCESS != res)
{
    // handle error
}
```
## Start
```c++
// Application
Result res = application.start();
if (Result::SUCCESS != res)
{
    // handle error
}
// Traffic generator
Result res = gen.start();
if (Result::SUCCESS != res)
{
    // handle error
}
```
## Stop
```c++
// Application
application.stop();
// Traffic generator
gen.stop();
```
## Deinitialization
```c++
// Application
application.deinitialization();
// Traffic generator
gen.deinitialization();
```
## Writing data (applicable for traffic generator only):
```
res = tg.writeData()
```