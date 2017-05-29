# Logger usage
It is an easy console/file logger.
## Configuration:
```c++
logger::Logger& l = logger::Logger::instance();
if (!l.configure())
{
    // handle error
}
```
## Initialization: 
```c++
logger::Logger& l = logger::Logger::instance();
if (!l.initialize())
{
    // handle error
}
```
## Deinitialization: 
```
logger::Logger& l = logger::Logger::instance();
l.deinitialize();
```