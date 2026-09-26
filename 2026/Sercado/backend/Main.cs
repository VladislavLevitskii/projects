using backend;
using backend.Crawler;
using Microsoft.Data.SqlClient;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.DependencyInjection;
using System.Text.Json;
using System.Text.Json.Serialization;

var builder = WebApplication.CreateBuilder(args);

var requiredEnvVars = new[] { "DB_HOST", "DB_PORT_INTERNAL", "DB_NAME", "DB_USER", "DB_PASSWORD" };

foreach (var envVar in requiredEnvVars)
{
    if (string.IsNullOrEmpty(builder.Configuration[envVar]))
    {
        throw new Exception($"Missing required environment variable: {envVar}");
    }
}

var sqlBuilder = new SqlConnectionStringBuilder
{
    DataSource = $"{builder.Configuration["DB_HOST"]},{builder.Configuration["DB_PORT_INTERNAL"]}",
    InitialCatalog = builder.Configuration["DB_NAME"],
    UserID = builder.Configuration["DB_USER"],
    Password = builder.Configuration["DB_PASSWORD"],
    
    TrustServerCertificate = true,
    Encrypt = true,
    MultipleActiveResultSets = true
};

var connectionString = sqlBuilder.ConnectionString;

builder.Services.AddPooledDbContextFactory<AppDbContext>(options =>
    options.UseSqlServer(connectionString));

builder.Services.ConfigureHttpJsonOptions(options =>
{
    options.SerializerOptions.Converters.Add(new JsonStringEnumConverter(JsonNamingPolicy.CamelCase));
});

builder.Services.AddHttpClient(string.Empty)
    .ConfigurePrimaryHttpMessageHandler(() => new HttpClientHandler
    {
        AllowAutoRedirect = true,
        MaxAutomaticRedirections = 5
    });

builder.Services.AddSingleton<CrawlQueue>();

builder.Services.AddScoped<CrawlerExecutor>();
builder.Services.AddScoped<ExecutionService>();

builder.Services.AddHostedService<SchedulerService>();

builder.Services
    .AddOpenApi()
    .AddGraphQLServer()
    .AddQueryType<Query>()
    .RegisterDbContextFactory<AppDbContext>();

var app = builder.Build();

using (var scope = app.Services.CreateScope())
{
    var factory = scope.ServiceProvider.GetRequiredService<IDbContextFactory<AppDbContext>>();
    using var context = factory.CreateDbContext();
    context.Database.Migrate();
}

app.MapGraphQL();

if (app.Environment.IsDevelopment())
{
    app.MapOpenApi();
}

app.UseHttpsRedirection();

RouteExtensions.AddRoutes(app);

app.Run();