using Microsoft.Extensions.DependencyInjection;

namespace Vista.SDK;

public static class DIExtensions
{
    public static IServiceCollection AddVIS(this IServiceCollection services)
    {
        services.AddSingleton<VIS>(VIS.Instance);
        services.AddSingleton<IVIS>(VIS.Instance);
        return services;
    }
}
