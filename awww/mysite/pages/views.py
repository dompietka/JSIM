from django.shortcuts import render
import datetime

def home(request):
    context = {
        "page_title": "Home",
        "heading": "Hello from Django!",
        "server_time": datetime.datetime.now(),
    }
    return render(request, "pages/home.html", context)

def about(request):
    skills = ["Python", "HTTP", "HTML", "CSS"]
    context = {
        "skills": skills
    }
    return render(request, "pages/about.html", context)

def greet(request, name):
    context = {
        "name": name
    }
    return render(request, "pages/greet.html", context)

ALL_PROJECTS = [
    {"name": "Socket Server",    "lang": "Python",     "year": 2025, "done": True},
    {"name": "HTML Profile",     "lang": "HTML",       "year": 2025, "done": True},
    {"name": "CSS Layout",       "lang": "CSS",        "year": 2025, "done": True},
    {"name": "Django App",       "lang": "Python",     "year": 2025, "done": False},
    {"name": "REST API",         "lang": "Python",     "year": 2024, "done": True},
    {"name": "React Dashboard",  "lang": "JavaScript", "year": 2024, "done": True},
    {"name": "SQL Queries Lab",  "lang": "SQL",        "year": 2024, "done": True},
    {"name": "CLI Tool",         "lang": "Python",     "year": 2023, "done": True},
]

def projects(request):
    q = request.GET.get("q", "").strip()  # get query, remove whitespace

    if q:
        # Filter projects where name or lang contains q (case-insensitive)
        project_list = [
            p for p in ALL_PROJECTS
            if q.lower() in p["name"].lower() or q.lower() in p["lang"].lower()
        ]
    else:
        project_list = ALL_PROJECTS

    done_count = len(project_list)  # number of filtered projects

    context = {
        "project_list": project_list,
        "done_count": done_count,
        "q": q,
    }

    return render(request, "pages/projects.html", context)