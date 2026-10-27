from django.test import TestCase

class PagesTests(TestCase):

    def test_home_status(self):
        response = self.client.get("/")
        self.assertEqual(response.status_code, 200)

    def test_about_uses_correct_template(self):
        response = self.client.get("/about/")
        self.assertTemplateUsed(response, "pages/about.html")

    def test_about_has_four_skills(self):
        response = self.client.get("/about/")
        self.assertEqual(len(response.context["skills"]), 4)

    def test_greet_contains_name(self):
        response = self.client.get("/greet/Alice/")
        self.assertIn(b"Alice", response.content)

    def test_projects_search_filters(self):
        response = self.client.get("/projects/?q=python")

        for project in response.context["project_list"]:
            self.assertTrue(
                project["lang"].lower() == "python"
                or "python" in project["name"].lower()
            )