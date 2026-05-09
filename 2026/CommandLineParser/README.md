# About
CommandLineParser is a C# library that provides a simple and elegant way to parse command-line arguments. It is designed to be as flexible as possible without sacrificing ease of use. You can seamlessly implement custom validators and type converters to write clean, reusable code.

# Features
* **Aliases:** Support for multiple names for the same argument (e.g., `-h` and `--help`).
* **Flexibility:** Support for required and optional arguments.
* **Safety:** Built-in and custom validators combined with type parsers.
* **Documentation:** Automatic help text generation via `HelpProvider`.
* **Extensibility:** Seamless conversion of string arguments to custom C# types.

# Requirements
* **.NET SDK 10.0** or later.
* **Git** for cloning the repository.
* (Optional) **DocFX** if you want to build the local HTML documentation.

# Design decisions

While this library aims to be flexible, it has specific design boundaries. 
Here is an overview of what is and is not supported:

## Support

- arguments can be passed to options either separated by a space `--port 8080` or using the inline assignment syntax `--port=8080`
- options can define and accept multiple arguments e.g., `--size 1920 1080`
- automatic dash prefixing: If an alias is defined without a leading dash (-), the builder automatically adds it

## Does NOT support

- variadic plain arguments (catch-all arguments)
- dynamic or unbounded plain arguments
    - every plain argument must be explicitly defined in the parser setup
- short flag bundling (e.g., combining `-a` and `-b` into `-ab`)
- mutually exclusive options (where specifying one option automatically forbids another)
  
## Building Documentation
This project includes an automated documentation setup using DocFX. To generate and view the API documentation locally, ensure you have DocFX installed, and then run the following command from the root of the repository:

```bash
docfx docs/docfx.json --serve
```

This will build the documentation and host it on a local web server (usually at http://localhost:8080).

# Installing library
Library is cross platform and can be used both on Linux and Windows.

## Linux

1. First thing you have to do is to download .NET package like this (command may differ from distro to distro):

```bash
sudo dnf install dotnet-sdk-10.0
```

2. Second thing is to clone this repository:  

```bash
git clone <URL-this-repo>
```

3. Adding dependency (you need to use relevant path to this .csproj):
```bash
dotnet add reference ../CommandLineParser/CommandLineParser.csproj
```

4. Building and running the project:
```bash
dotnet build
dotnet run
```

## Windows

1. First thing you have to do is to download .NET package from official website

2. Second thing is to clone this repository from terminal:  

```powershell
git clone <URL-this-repo>
```

3. There is two ways how to add dependency to your project:

    1. use Visual Studio 22(26):
        - right click on your solution -> Add -> Existing Project
        - right click on project you want to add dependency and again Add -> Project reference.
        - then you can run your project with our library, by clicking green Run button.
    2. using terminal:
    ```powershell
    dotnet add reference ../CommandLineParser/CommandLineParser.csproj
    dotnet build
    dotnet run
    ```

# Usage

Library provides this functionality:

## CommandParser

This class is core of the whole library:

```csharp
CommandParser parser;
```

For using this class you have to create helper class `CommandParserBuilder` and build it:

```csharp
parser = new CommandParserBuilder(plainArgsDelimiter: "--");
```

This `CommandParserBuilder` has 1 optional argument `plainArgsDelimiter`, with default "--" respectively.

Last thing you have to do is **building**, you have to call CommandParserBuilder's method `.Build()`.

Additionally, you can automatically generate a help message by calling `GetHelp()`. 
This returns a HelpProvider ([here](#helpprovider)) that contains all help messages for options and parameters.
Moreover you can create your own generation of help message (tutorial is [here](#extend-help-message)).

Although it is not quite useful to have such instance without any options, so let's introduce them.

## OptionDefinition

This class is here to bring our soulless `CommandParser` some functionality.

```csharp
OptionDefinition helpOptionDef;
```

And again, there is class for building `OptionDefinition` **OptionDefinitionBuilder**:

```csharp
helpOptionDef = new OptionDefinitionBuilder("help");
```

This constructor has only one string argument by which you can "name" your option. This will be used in actual using your options. And again you have to **build** it: `.Build()`.

By now you created an option but not added to `CommandParser`, you can do it by calling `WithOption()` which takes your option instance:

```csharp
parser = new CommandParserBuilder().WithOption(helpOptionDef);
```

### Aliases

For actual using this option in command line you need to define aliases by calling `WithAlias()`. Function has variadic number of arguments, this means you can add any number of aliases of your choice. However it should be without any spaces (or your defined delimiter).

```csharp
helpOptionDef = new OptionDefinitionBuilder("help").WithAlias("--help", "--literally_anything");
```
---
**Note:**
Having the same alias for different options will raise 
`CommandLineParserAliasException`.

**Note:**
If you add an alias without the dash in front, the dash will be automatically added.
---

## Optional extensions

This sections describes some methods you can call to add/improve your options and parser.

### HelpProvider

This class is used to (unexpectedly) provide help for other users, by calling --help (or other argument you can define for calling help):

```csharp
new HelpProvider(
    longHelpText: "time [options] command arguments...]", 
    shortHelpText: "time command"
);
```

And this class can be used not only for program (parser) itself but even for options, you can add this feature by calling `WithHelp()` with argument `HelpProvider`:

```csharp
parser = new CommandParserBuilder()
.WithHelp(new HelpProvider(
    longHelpText: "time [options] command [arguments...]",
    shortHelpText: "time command"
));
```

or OptionDefinitionBuilder's `WithHelp()`:

```csharp
helpOptionDef = new OptionDefinitionBuilder("help")
.WithHelp(
new HelpProvider(
    longHelpText: "Print a usage message on standard output and exit successfully.", 
    shortHelpText: "Print help."));
```

### Parameters

For adding parameters to your options you can use builder's method 

```csharp
AddParameter<T>(
    string name, 
    Func<ParameterDefinitionBuilder<T>, 
    ParameterDefinition<T>> configure)
```

For that you need to create your own `ParameterDefinition`, which uses some type (usually string) for help and the most important thing:
you can use **lambda** notation (see in example below). That lambda should also be built by `Build()` and has some self-explanatory methods:

- `IsRequired` - only 1 argument bool
- `WithHelp`- takes well-known HelpProvider
- `WithParser` - takes some user-defined string parser 
    - list of default parsers:
        - `bool`
        - `int`
        - `string`
        - `enum`
        - `lambda`

    every parser has method `Parse` that takes argument (string) and returns corresponding object.
---
**Note:**

Always call `WithParser()` for your parameters when using type that doesn't have default parser. Otherwise, you will get an exception when trying to parse command line with that parameter.

---
- `WithValidation` - takes some user-defined string validator 
    - list of default validators:
        - `bool`
        - `int`
        - `string`
        - `enum`
        - `lambda`

    every validator has method `Validate` that takes argument (string) and returns bool

Simple example of adding argument:

```csharp
.AddParameter<string>("FILE", param => param
    .IsRequired()
    .WithHelp(new HelpProvider(longHelpText: "The target file path.", shortHelpText: "File path."))
    .WithValidation(new DefaultValidators.StringValidator(
        regexPattern: @".*\.txt$", 
        minLength: 5
    ))
    .WithParser(new DefaultParsers.StringParser())
    .WithDefault("results.txt")
    .Build(out fileParamDef)
)
```

---
**Note:**

Note that `Build()` can take out `ParameterDefinition` and save it there.

---
**Note:**

Order in which you add parameters is important, because you can access them by index in `CommandParserResult`.

---

### Plain Arguments

Plain arguments are arguments that are associated with whole command, not with any option. They use same class as `OptionParameters` and are part of default option that is called "plainArgs".

Example:
```csharp
options.parser = new CommandParserBuilder(plainArgsDelimiter: "--")
.WithPlainArg<string>("command", param => param
    .IsRequired()
    .WithHelp(new HelpProvider(
        longHelpText: "The command to run and time.",
        shortHelpText: "Target command."
    ))
    .WithValidation(new DefaultValidators.StringValidator())
    .WithParser(new DefaultParsers.StringParser())
    .Build()
)
.Build();
```

### Custom Validators and Parsers

If the default validators and parsers lack some functionality not achievable by lambda parsers and validators, you might want to create your own custom validator or parser. For that you need to implement interfaces `IValidator<T>` or `IParser<T>`.
You should use `IValidator<T>` only if you need additional information passed to validator (e.g. regex pattern for string validator), otherwise you can use `LambdaValidator`, which doesn't require creating new class. Same applies for parsers with `LambdaParser`.

For example, if you want to create custom parser for int:
```csharp
public sealed class CustomParser : IParser<MyCustomClass>
{
    public MyCustomClass Parse(string value) 
    {
        // Your parsing logic here
        return new MyCustomClass(value);
    }
}
```
```csharp
public sealed class MyCustomClassValidator : IValidator
{
    
    public MyCustomClassValidator() { throw new NotImplementedException(); }
    public bool Validate(string value) { throw new NotImplementedException(); }
}
```
## Parsing and using CommandParses

Now after creating and defining our parses it is time to use it.

Function `Parse()` of `CommandParser` that takes string (usually whole line) and returns object `CommandParserResult`.

That object `CommandParserResult` can be used to find out which options were used with what parameters and so on.

For that there is methods like:

- `GetOption()`: which can take either string with `name` (remember that argument of option's builder) or that variable where you store object OptionDefinition.

    Returns result of parsing command-line options, including validation status and access to parsed. 
    And that object (`CommandParserOptionResult`) has only one self-explanatory method `GetArg<T>()`,
    (expanded with overloading by `string` and `ParameterDefinition`)

- `HasOption()`: has the same arguments but returns bool instead of `CommandParserOptionResult`.
- `GetPlainArg()`: Retrieves a parsed plain argument (arguments not associated with any option)
    - it takes exactly one parameter the `ParameterDefinition<T>` variable that you saved during the parser building phase

## Error Handling & Exceptions

The library uses a specific hierarchy of exceptions to help you identify what went wrong. 
All exceptions inherit from the base `CommandLineParserException`.

### Configuration Errors (Thrown by Builder)
These exceptions are thrown immediately when calling `CommandParserBuilder.Build()` if your setup is invalid. You should fix these during development.

- `CommandLineParserAliasException`: Thrown when you define duplicate aliases for options (e.g., assigning `-v` to both `--version` and `--verbose`).
- `CommandLineParserException`: Thrown for other configuration issues, such as duplicate parameter names across your definitions.

### Parsing Errors (Returned in Result)
During the actual parsing of user input via `CommandParser.Parse()`, the library does **not** crash the application. Instead, it catches the following exceptions and stores them in the `CommandParserResult.Error` property.

- `CommandLineParserParsingException`: Thrown when a string from the command line cannot be converted to the target type, or when a **required** parameter/option is missing.
- `CommandLineParserValidationException`: Thrown when the parsed value successfully converts to the target type, but fails your custom `IValidator<T>` rules (e.g., integer is out of bounds, string is too short).

## Extend help message

Because we think that every program can have some non-trivial context, we added some extensibility to `HelpProvider`. This is the only public class that is not sealed and here is an simple example how to 
extend the functionality:

This **MUST** be done in custom HelpProvider, constructor **MUST** have exactly this 3 parameters, which will add to your lists informations about options/arguments:

```csharp
private readonly List<OptionHelpInfo> _options;
private readonly List<ParameterHelpInfo> _arguments;

public SimpleHelpProvider(
    List<OptionHelpInfo> options, 
    List<ParameterHelpInfo> arguments, 
    ProgramHelpInfo programHelp) 
    : base(programHelp.longDescription, programHelp.shortDescription)
{
    _options = options;
    _arguments = arguments;
}

public override string GetLongHelpText()
{ /* your implementation */ }

public override string GetShortHelpText()
{ /* your implementation */ }
```

Obviously you can change names of the variables.

This HelpInfo records look like this:

```csharp
    public sealed record OptionHelpInfo(
        string name, 
        string longDescription, 
        string shortDescription, 
        IReadOnlyList<string> aliases,
        IReadOnlyList<ParameterHelpInfo> parameterNames
    );
    
    public sealed record ParameterHelpInfo(string name, string longDescription, string shortDescription);
    public sealed record ProgramHelpInfo(string longDescription, string shortDescription);
```

So in overridden functions you can create something like this:

```csharp
foreach (var opt in _options)
{
    sb.AppendLine($"Name: {opt.name}");
}
```

And this is the same messages you inserted during initialization of the parser.

To add this custom help provider you need to call `WithCustomHelp` on parser builder, adding this function:

```csharp
.WithCustomHelp((opts, args, progInfo) => new SimpleHelpProvider(opts, args, progInfo));
```

It is not important how you name these variables (opts, args, progInfo).

Some simple example of using custom help provider you can find in [example project](./ExampleProject/src/Program.cs).

## Tips

It might be useful to store the references to your options and parameters, for example in a class `ParserSchema`:

```csharp
public class ParserSchema
{
    public CommandParser parser = null!;

    public OptionDefinition plainArgsDef = null!;
    
    public OptionDefinition formatOptionDef = null!;
    public OptionDefinition outputOptionDef = null!;
    public OptionDefinition portabilityOptionDef = null!;
    public OptionDefinition appendOptionDef = null!;
    public OptionDefinition verboseOptionDef = null!;
    public OptionDefinition helpOptionDef = null!;
    public OptionDefinition versionOptionDef = null!;

    public ParameterDefinition<string> fileParamDef = null!;
}
```

And use function like **InitParse(ParserSchema options)**:

```csharp
public static void InitParse(ParserSchema options)
{
    options.formatOptionDef = new OptionDefinitionBuilder("format")
        .WithHelp(new HelpProvider(
            longHelpText: "...",
            shortHelpText: "..."
        ))
        .WithAlias("-f", "--format")
        .AddParameter<string>(/*some argument*/)
        .Build();

    /* Other options... */

    options.parser = new CommandParserBuilder(plainArgsDelimiter: "--")
        .WithHelp(new HelpProvider(
            longHelpText: "...",
            shortHelpText: "..."
        ))
        .WithOption(options.formatOptionDef)

        /* every option */

        .WithOption(options.versionOptionDef)
        // Applying the same lambda pattern to the plain arguments
        .WithPlainArg<T>(/*some argument*/)
        .Build();

    options.plainArgsDef = options.parser.PlainArgs;
}
```

And finally some usage of options:

```csharp

while (line.Length > 0)
{
    CommandParserResult result = parserSchema.parser.Parse(line);

    if (result)
    {
        if (result.GetOption("help"))
        {
            Console.WriteLine("Mock Output: Printing Help Message...");
            line = Console.ReadLine()?.Split() ?? Array.Empty<string>();
            continue;
        }
        if (result.HasOption(parserSchema.versionOptionDef))
        {
            Console.WriteLine("Mock Output: GNU time version 1.9 (mock)");
            line = Console.ReadLine()?.Split() ?? Array.Empty<string>();
            continue;
        }
        var outputResult = result.GetOption(parserSchema.outputOptionDef);

        if (outputResult)
        {
            string mode = result.GetOption(parserSchema.appendOptionDef) ? "Appending to" : "Overwriting";
            // Retrieved by definition reference
            Console.WriteLine($"* Output Route: {mode} file '{outputResult.GetArg(parserSchema.fileParamDef)}'");
        }
        /* and so on */
```

### Example

You can find whole example [here](./ExampleProject/src/Program.cs)

## Conclusion

We hope this library makes designing command-line interfaces in your applications easier and more enjoyable!

Contact us if you find any bugs or if you have design suggestions to improve this library.

[Martin Komárek](https://github.com/TheFallENSteel) - co-author
[Vladislav Levitskii](https://github.com/VladislavLevitskii/) - co-author

