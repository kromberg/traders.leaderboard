# Logger usage
It is an easy console logger.
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

# Leaderboard application
## Initialization
```c++
app::Application& application = app::Application::getInstance();
Result res = application.initialize();
if (Result::SUCCESS != res)
{
	// handle error
}
```
ц
