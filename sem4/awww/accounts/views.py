from django.shortcuts import render, redirect
from django.contrib.auth import login
from .forms import RegisterForm
from django.contrib.auth.decorators import login_required
from django.contrib.auth.decorators import login_required, user_passes_test
from django.shortcuts import render, redirect


def register(request):
    form = RegisterForm()
    if request.method == "POST":
        form = RegisterForm(request.POST)
        if form.is_valid():
            user = form.save()
            login(request, user) 
            return redirect("home")
    return render(request, "accounts/register.html", {"form": form})

@login_required
def profile(request):
    return render(request, "accounts/profile.html", {"user": request.user})

def logout(request):
    logout(request)
    return redirect("home")

def login(request):
    return render(request, "accounts/login.html")

