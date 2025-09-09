using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace MikoInstaller.Hook;

public class UseState<T> : INotifyPropertyChanged
{
    private T _value;
    
    public UseState(T initialValue)
    {
        _value = initialValue;
    }
    
    public T Value
    {
        get => _value;
        set
        {
            if (!Equals(_value, value))
            {
                _value = value;
                OnPropertyChanged();
                if (StateChanged != null)
                    StateChanged.Invoke(_value);
            }
        }
    }
    
    public event Action<T> StateChanged;
    public event PropertyChangedEventHandler PropertyChanged;
    
    protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
    {
        if (PropertyChanged != null)
            PropertyChanged.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
    
    public void SetState(T newValue)
    {
        Value = newValue;
    }
    
    public void SetState(Func<T, T> updater)
    {
        Value = updater(_value);
    }
    
    public static implicit operator T(UseState<T> useState)
    {
        return useState.Value;
    }
}